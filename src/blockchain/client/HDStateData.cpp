// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"

#include "internal/api/Api.hpp"
#include "internal/blockchain/Blockchain.hpp"

#include "HDStateData.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::HDStateData::"

namespace opentxs::blockchain::client::implementation
{
HDStateData::HDStateData(
    const internal::Network& network,
    const WalletDatabase& db,
    const api::client::blockchain::HD& node,
    const filter::Type filter,
    const Subchain subchain) noexcept
    : network_(network)
    , db_(db)
    , node_(node)
    , filter_type_(filter)
    , subchain_(subchain)
    , running_(false)
    , last_indexed_()
    , last_scanned_()
    , blocks_to_request_()
    , outstanding_blocks_()
    , process_block_queue_()
{
}

HDStateData::HDStateData(HDStateData&& rhs) noexcept
    : network_(rhs.network_)
    , db_(rhs.db_)
    , node_(rhs.node_)
    , filter_type_(rhs.filter_type_)
    , subchain_(rhs.subchain_)
    , running_(rhs.running_.load())
    , last_indexed_(std::move(rhs.last_indexed_))
    , last_scanned_(std::move(rhs.last_scanned_))
    , blocks_to_request_(std::move(rhs.blocks_to_request_))
    , outstanding_blocks_(std::move(rhs.outstanding_blocks_))
    , process_block_queue_(std::move(rhs.process_block_queue_))
{
}

auto HDStateData::get_targets(
    const internal::WalletDatabase::Patterns& keys,
    const std::vector<internal::WalletDatabase::UTXO>& unspent) const noexcept
    -> blockchain::internal::GCS::Targets
{
    auto output = blockchain::internal::GCS::Targets{};
    std::transform(
        std::begin(keys),
        std::end(keys),
        std::back_inserter(output),
        [](const auto& in) { return reader(in.second); });

    switch (filter_type_) {
        case filter::Type::Basic_BIP158: {
            std::transform(
                std::begin(unspent),
                std::end(unspent),
                std::back_inserter(output),
                [](const auto& in) { return in.second.script(); });
        } break;
        case filter::Type::Basic_BCHVariant:
        case filter::Type::Extended_opentxs:
        default: {
            std::transform(
                std::begin(unspent),
                std::end(unspent),
                std::back_inserter(output),
                [](const auto& in) { return in.first.Bytes(); });
        }
    }

    return output;
}

auto HDStateData::index() noexcept -> void
{
    const auto first =
        last_indexed_.has_value() ? last_indexed_.value() + 1 : Bip32Index{0};
    const auto last = node_.LastGenerated(subchain_);
    auto elements = WalletDatabase::ElementMap{};

    for (auto i{first}; i <= last; ++i) {
        const auto& element = node_.BalanceElement(subchain_, i);
        index_element(filter_type_, element, i, elements);
    }

    db_.SubchainAddElements(node_.ID(), subchain_, filter_type_, elements);
}

auto HDStateData::index_element(
    const filter::Type type,
    const api::client::blockchain::BalanceNode::Element& input,
    const Bip32Index index,
    WalletDatabase::ElementMap& output) noexcept -> void
{
    const auto pubkeyHash = input.PubkeyHash();
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Indexing public key with hash ")(
        pubkeyHash->asHex())
        .Flush();
    auto& list = output[index];
    auto scripts = std::vector<std::unique_ptr<const block::bitcoin::Script>>{};
    scripts.reserve(4);  // WARNING keep this number up to date if new scripts
                         // are added
    const auto& p2pk =
        scripts.emplace_back(network_.API().Factory().BitcoinScriptP2PK(
            network_.Chain(), *input.Key()));
    const auto& p2pkh =
        scripts.emplace_back(network_.API().Factory().BitcoinScriptP2PKH(
            network_.Chain(), *input.Key()));
    const auto& p2sh_p2pk = scripts.emplace_back(
        network_.API().Factory().BitcoinScriptP2SH(network_.Chain(), *p2pk));
    const auto& p2sh_p2pkh = scripts.emplace_back(
        network_.API().Factory().BitcoinScriptP2SH(network_.Chain(), *p2pkh));

    OT_ASSERT(p2pk);
    OT_ASSERT(p2pkh);
    OT_ASSERT(p2sh_p2pk);
    OT_ASSERT(p2sh_p2pkh);

    switch (type) {
        case filter::Type::Extended_opentxs: {
            OT_ASSERT(p2pk->Pubkey().has_value());
            OT_ASSERT(p2pkh->PubkeyHash().has_value());
            OT_ASSERT(p2sh_p2pk->ScriptHash().has_value());
            OT_ASSERT(p2sh_p2pkh->ScriptHash().has_value());

            list.emplace_back(space(p2pk->Pubkey().value()));
            list.emplace_back(space(p2pkh->PubkeyHash().value()));
            list.emplace_back(space(p2sh_p2pk->ScriptHash().value()));
            list.emplace_back(space(p2sh_p2pkh->ScriptHash().value()));
        } break;
        case filter::Type::Basic_BIP158:
        case filter::Type::Basic_BCHVariant:
        default: {
            for (const auto& script : scripts) {
                script->Serialize(writer(list.emplace_back()));
            }
        }
    }
}

auto HDStateData::process() noexcept -> void
{
    struct Cleanup {
        Cleanup(HDStateData::OutstandingMap::iterator it, HDStateData& data)
            : it_(it)
            , data_(data)
        {
        }

        ~Cleanup()
        {
            data_.outstanding_blocks_.erase(it_);
            data_.process_block_queue_.pop();
        }

    private:
        HDStateData::OutstandingMap::iterator it_;
        HDStateData& data_;
    };

    const auto start = Clock::now();
    const auto& filters = network_.FilterOracle();
    auto it = process_block_queue_.front();
    const auto& blockHash = it->first.get();
    auto cleanup = Cleanup{it, *this};
    const auto pBlock = it->second.get();

    if (false == bool(pBlock)) {
        LogVerbose("opentxs::blockchain::client::internal::")(__FUNCTION__)(
            ": Invalid block ")(blockHash.asHex())
            .Flush();
        auto& vector = blocks_to_request_;
        vector.emplace(vector.begin(), blockHash);

        return;
    }

    const auto& block = *pBlock;
    auto tested = WalletDatabase::MatchingIndices{};
    auto patterns = blockchain::internal::GCS::Targets{};
    auto elements = db_.GetUntestedPatterns(
        node_.ID(), subchain_, filter_type_, blockHash.Bytes());
    std::for_each(
        std::begin(elements), std::end(elements), [&](const auto& pattern) {
            const auto& [id, data] = pattern;
            const auto& [index, subchain] = id;
            tested.emplace_back(index);
            patterns.emplace_back(reader(data));
        });

    const auto pFilter = filters.LoadFilter(filter_type_, blockHash);

    OT_ASSERT(pFilter);

    const auto& filter = *pFilter;
    auto potential = WalletDatabase::Patterns{};

    for (const auto& it : filter.Match(patterns)) {
        const auto pos = std::distance(patterns.cbegin(), it);
        auto& [id, element] = elements.at(pos);
        potential.emplace_back(std::move(id), std::move(element));
    }

    const auto confirmed = block.FindMatches(filter_type_, {}, potential);
    const auto& oracle = network_.HeaderOracle();
    const auto pHeader = oracle.LoadHeader(blockHash);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;
    update_utxos(block, header.Position(), confirmed);
    const auto [balance, unconfirmed] = db_.GetBalance();
    LogOutput("opentxs::blockchain::client::internal::")(__FUNCTION__)(
        ": block ")(blockHash.asHex())(" processed in ")(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start)
            .count())(" milliseconds. ")(confirmed.size())(" of ")(
        potential.size())(" potential matches confirmed. Wallet balance is: ")(
        unconfirmed)(" (")(balance)(" confirmed)")
        .Flush();
    db_.SubchainMatchBlock(
        node_.ID(), subchain_, filter_type_, tested, blockHash.Bytes());

    if (0 < confirmed.size()) {
        // Re-scan the last 1000 blocks
        const auto height = std::max(header.Height() - 1000, block::Height{0});
        last_scanned_ = block::Position{height, oracle.BestHash(height)};
    }
}

auto HDStateData::scan() noexcept -> void
{
    const auto start = Clock::now();
    const auto& headers = network_.HeaderOracle();
    const auto& filters = network_.FilterOracle();
    const auto best = headers.BestChain();
    const auto startHeight = std::min(
        best.first,
        last_scanned_.has_value() ? last_scanned_.value().first + 1 : 0);
    const auto first =
        last_scanned_.has_value()
            ? block::Position{startHeight, headers.BestHash(startHeight)}
            : block::Position{1, headers.BestHash(1)};
    const auto stopHeight = std::min(startHeight + 9999, best.first);

    if (first.second->empty()) { return; }  // Reorg occured while processing

    const auto elements = db_.GetPatterns(node_.ID(), subchain_, filter_type_);
    const auto utxos = db_.GetUnspentOutputs();
    auto highestTested = last_scanned_.value_or(
        make_blank<block::Position>::value(network_.API()));
    auto atLeastOnce{false};

    for (auto i{startHeight}; i <= stopHeight; ++i) {
        auto blockHash = headers.BestHash(i);
        const auto pFilter = filters.LoadFilter(filter_type_, blockHash);

        if (false == bool(pFilter)) { break; }

        atLeastOnce = true;
        highestTested.first = i;
        highestTested.second = blockHash;
        const auto& filter = *pFilter;
        auto patterns = get_targets(elements, utxos);
        auto matches = filter.Match(patterns);
        const auto size{matches.size()};

        if (0 < matches.size()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": GCS for block ")(
                blockHash->asHex())(" at height ")(i)(
                " matches at least one of the ")(patterns.size())(
                " target elements for this subchain")
                .Flush();
            const auto retest = db_.GetUntestedPatterns(
                node_.ID(), subchain_, filter_type_, blockHash->Bytes());
            patterns = get_targets(retest, utxos);
            matches = filter.Match(patterns);
            LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(matches.size())(" of ")(
                size)(" matches are new")
                .Flush();

            if (0 < matches.size()) {
                blocks_to_request_.emplace_back(std::move(blockHash));
            }
        }
    }

    if (atLeastOnce) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Found ")(
            blocks_to_request_.size())(" potential matches between blocks ")(
            startHeight)(" and ")(highestTested.first)(" in ")(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                Clock::now() - start)
                .count())(" milliseconds")
            .Flush();
        last_scanned_ = std::move(highestTested);
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Missing filter for block at height ")(startHeight)
            .Flush();
    }
}

auto HDStateData::update_utxos(
    const block::bitcoin::Block& block,
    const block::Position& position,
    const block::Block::Matches matches) noexcept -> void
{
    auto transactions = std::map<
        block::pTxid,
        std::pair<
            std::vector<Bip32Index>,
            const block::bitcoin::Transaction*>>{};

    for (const auto& [txid, elementID] : matches) {
        const auto& [index, subchainID] = elementID;
        const auto& [subchain, accountID] = subchainID;
        const auto& element = node_.BalanceElement(subchain, index);
        const auto& pTransaction = block.at(txid->Bytes());

        OT_ASSERT(pTransaction);

        auto& arg = transactions[txid];
        auto& [outputs, pTX] = arg;
        const auto& transaction = *pTransaction;
        auto i = Bip32Index{0};

        for (const auto& output : transaction.Outputs()) {
            const auto& script = output.Script();

            switch (script.Type()) {
                case block::bitcoin::Script::Pattern::PayToPubkey: {
                    const auto pKey = element.Key();

                    OT_ASSERT(pKey);
                    OT_ASSERT(script.Pubkey().has_value());

                    const auto& key = *pKey;

                    if (key.PublicKey() == script.Pubkey().value()) {
                        outputs.emplace_back(i);

                        if (nullptr == pTX) { pTX = pTransaction.get(); }
                    }

                    // TODO mark key as used
                } break;
                case block::bitcoin::Script::Pattern::PayToPubkeyHash: {
                    const auto hash = element.PubkeyHash();

                    OT_ASSERT(script.PubkeyHash().has_value());

                    if (hash->Bytes() == script.PubkeyHash().value()) {
                        outputs.emplace_back(i);

                        if (nullptr == pTX) { pTX = pTransaction.get(); }
                    }

                    // TODO mark key as used
                } break;
                case block::bitcoin::Script::Pattern::PayToScriptHash:
                case block::bitcoin::Script::Pattern::PayToMultisig:
                default: {
                }
            };

            ++i;
        }
    }

    auto unspent = std::set<blockchain::block::bitcoin::Outpoint>{};

    for (const auto& utxo : db_.GetUnspentOutputs()) {
        unspent.insert(utxo.first);
    }

    for (const auto& transaction : block) {
        OT_ASSERT(transaction);

        for (const auto& input : transaction->Inputs()) {
            if (0 < unspent.count(input.PreviousOutput())) {
                auto& pTx = transactions[transaction->ID()].second;

                if (nullptr == pTx) { pTx = transaction.get(); }
            }
        }
    }

    for (const auto& [txid, data] : transactions) {
        auto& [outputs, pTX] = data;
        auto updated = db_.AddConfirmedTransaction(position, outputs, *pTX);

        OT_ASSERT(updated);  // TODO handle database errors
    }
}
}  // namespace opentxs::blockchain::client::implementation
