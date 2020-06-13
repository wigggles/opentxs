// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "blockchain/database/Wallet.hpp"  // IWYU pragma: associated

#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <algorithm>
#include <iterator>
#include <map>
#include <numeric>
#include <tuple>
#include <type_traits>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

// #define OT_METHOD "opentxs::blockchain::database::Wallet::"

namespace opentxs::blockchain::database
{
Wallet::Wallet(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const blockchain::Type chain) noexcept
    : api_(api)
    , blockchain_(blockchain)
    , chain_(chain)
    , lock_()
    , patterns_()
    , subchain_pattern_index_()
    , subchain_last_indexed_()
    , subchain_version_()
    , subchain_last_scanned_()
    , subchain_last_processed_()
    , match_index_()
    , outputs_()
    , unconfirmed_new_()
    , confirmed_new_()
    , unconfirmed_spend_()
    , confirmed_spend_()
    , orphaned_new_()
    , orphaned_spend_()
    , transactions_()
    , tx_to_block_()
    , block_to_tx_()
{
}

auto Wallet::add_transaction(
    const Lock& lock,
    const block::Hash& block,
    const block::bitcoin::Transaction& transaction) const noexcept -> bool
{
    if (auto it = transactions_.find(transaction.ID());
        transactions_.end() != it) {
        auto& serialized = it->second;
        transaction.MergeMetadata(serialized);
        auto updated = transaction.Serialize();

        OT_ASSERT(updated.has_value());

        serialized = std::move(updated.value());
    } else {
        auto serialized = transaction.Serialize();

        OT_ASSERT(serialized.has_value());

        transactions_.emplace(transaction.ID(), std::move(serialized.value()));
    }

    {
        auto& index = tx_to_block_[transaction.ID()];
        index.emplace_back(block);
        dedup(index);
    }

    {
        auto& index = block_to_tx_[block];
        index.emplace_back(transaction.ID());
        dedup(index);
    }

    return true;
}

auto Wallet::AddConfirmedTransaction(
    const block::Position& block,
    const std::vector<std::uint32_t> outputIndices,
    const block::bitcoin::Transaction& transaction) const noexcept -> bool
{
    Lock lock(lock_);
    const auto& [height, blockHash] = block;

    if (false == add_transaction(lock, blockHash, transaction)) {
        LogOutput(__FUNCTION__)(": Error adding transaction to database")
            .Flush();

        return false;
    }

    for (const auto& input : transaction.Inputs()) {
        const auto& outpoint = input.PreviousOutput();

        if (auto out = find_output(lock, outpoint); out.has_value()) {
            auto& lastHeight = out.value()->second.first;

            if (false ==
                change_state(
                    lock, outpoint, lastHeight, height, confirmed_spend_)) {
                LogOutput(__FUNCTION__)(
                    ": Error updating consumed output state")
                    .Flush();

                return false;
            }

            lastHeight = height;
        }

        // NOTE consider the case of parallel chain scanning where one
        // transaction spends inputs that belong to two different subchains.
        // The first subchain to find the transaction will recognize the inputs
        // belonging to itself but might miss the inputs belonging to the other
        // subchain if the other subchain's scanning process has not yet
        // discovered those outputs.
        // This is fine. The other scanning process will parse this transaction
        // again and at that point all inputs will be recognized. The only
        // impact is that net balance change of the transaction will
        // underestimated temporarily until scanning is complete for all
        // subchains.
    }

    for (const auto index : outputIndices) {
        const auto outpoint =
            block::bitcoin::Outpoint{transaction.ID().Bytes(), index};

        if (auto out = find_output(lock, outpoint); out.has_value()) {
            auto& lastHeight = out.value()->second.first;

            if (false ==
                change_state(
                    lock, outpoint, lastHeight, height, confirmed_new_)) {
                LogOutput(__FUNCTION__)(": Error updating created output state")
                    .Flush();

                return false;
            }

            lastHeight = height;
        } else {
            const auto& output = transaction.Outputs().at(index);
            auto serialized = block::bitcoin::Output::SerializeType{};
            output.Serialize(serialized);
            outputs_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(outpoint),
                std::forward_as_tuple(height, std::move(serialized)));
            auto& map = confirmed_new_[height];
            map.emplace_back(outpoint);
            dedup(map);
        }
    }

    blockchain_.UpdateBalance(chain_, get_balance(lock));

    return true;
}

auto Wallet::change_state(
    const Lock& lock,
    const block::bitcoin::Outpoint& id,
    const block::Height originalHeight,
    const block::Height newHeight,
    OutputStateMap& to) const noexcept -> bool
{
    auto in = std::vector<OutputStateMap*>{};

    if (&to != &unconfirmed_new_) { in.emplace_back(&unconfirmed_new_); }

    if (&to != &confirmed_new_) { in.emplace_back(&confirmed_new_); }

    if (&to != &unconfirmed_spend_) { in.emplace_back(&unconfirmed_spend_); }

    if (&to != &confirmed_spend_) { in.emplace_back(&confirmed_spend_); }

    if (&to != &orphaned_new_) { in.emplace_back(&orphaned_new_); }

    if (&to != &orphaned_spend_) { in.emplace_back(&orphaned_spend_); }

    for (auto& from : in) {
        if (false == remove_state(lock, id, originalHeight, *from)) {
            LogOutput(__FUNCTION__)(": Error updating output state").Flush();

            return false;
        }
    }

    auto& map = to[newHeight];
    map.emplace_back(id);
    dedup(map);

    return true;
}

auto Wallet::find_output(const Lock& lock, const block::bitcoin::Outpoint& id)
    const noexcept -> std::optional<OutputMap::iterator>
{
    auto result = outputs_.find(id);

    if (outputs_.end() == result) {

        return {};
    } else {

        return result;
    }
}

auto Wallet::get_balance(const Lock&) const noexcept -> Balance
{
    auto output = Balance{};
    auto& [confirmed, unconfirmed] = output;
    auto cb = [this](const auto previous, const auto& in) -> auto
    {
        const auto& outpoints = in.second;
        auto cb = [this](const auto previous, const auto& in) -> auto
        {
            return previous + outputs_.at(in).second.value();
        };

        return std::accumulate(
            std::begin(outpoints), std::end(outpoints), previous, cb);
    };
    confirmed = std::accumulate(
        std::begin(confirmed_new_),
        std::end(confirmed_new_),
        std::uint64_t{0},
        cb);
    unconfirmed = std::accumulate(
        std::begin(unconfirmed_new_),
        std::end(unconfirmed_new_),
        confirmed,
        cb);

    return output;
}

auto Wallet::get_patterns(
    const Lock& lock,
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept(false) -> const IDSet&
{
    return subchain_pattern_index_.at(
        subchain_id(balanceNode, subchain, type, version));
}

auto Wallet::GetBalance() const noexcept -> Balance
{
    Lock lock(lock_);

    return get_balance(lock);
}

auto Wallet::GetPatterns(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> Patterns
{
    Lock lock(lock_);

    try {
        const auto& patterns =
            get_patterns(lock, balanceNode, subchain, type, version);

        return load_patterns(lock, balanceNode, subchain, patterns);
    } catch (...) {

        return {};
    }
}

auto Wallet::GetUnspentOutputs() const noexcept -> std::vector<UTXO>
{
    Lock lock(lock_);

    return get_unspent_outputs(lock);
}

auto Wallet::get_unspent_outputs(const Lock& lock) const noexcept
    -> std::vector<UTXO>
{
    auto retrieve = std::vector<block::bitcoin::Outpoint>{};

    for (const auto& [height, outpoints] : unconfirmed_new_) {
        retrieve.insert(retrieve.end(), outpoints.begin(), outpoints.end());
    }

    for (const auto& [height, outpoints] : confirmed_new_) {
        retrieve.insert(retrieve.end(), outpoints.begin(), outpoints.end());
    }

    for (const auto& [height, outpoints] : unconfirmed_spend_) {
        retrieve.insert(retrieve.end(), outpoints.begin(), outpoints.end());
    }

    dedup(retrieve);
    auto output = std::vector<UTXO>{};

    for (const auto& outpoint : retrieve) {
        const auto& [height, txout] = outputs_.at(outpoint);
        output.emplace_back(outpoint, txout);
    }

    return output;
}

auto Wallet::GetUntestedPatterns(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const ReadView blockID,
    const VersionNumber version) const noexcept -> Patterns
{
    Lock lock(lock_);

    try {
        const auto& allPatterns =
            get_patterns(lock, balanceNode, subchain, type, version);
        auto effectiveIDs = std::vector<pPatternID>{};

        try {
            const auto& matchedPatterns =
                match_index_.at(api_.Factory().Data(blockID));
            std::set_difference(
                std::begin(allPatterns),
                std::end(allPatterns),
                std::begin(matchedPatterns),
                std::end(matchedPatterns),
                std::back_inserter(effectiveIDs));
        } catch (...) {

            return load_patterns(lock, balanceNode, subchain, allPatterns);
        }

        return load_patterns(lock, balanceNode, subchain, effectiveIDs);
    } catch (...) {

        return {};
    }
}

auto Wallet::pattern_id(const SubchainID& subchain, const Bip32Index index)
    const noexcept -> pPatternID
{
    auto preimage = OTData{subchain};
    preimage->Concatenate(&index, sizeof(index));
    auto output = api_.Factory().Identifier();
    output->CalculateDigest(preimage->Bytes());

    return output;
}

auto Wallet::remove_state(
    const Lock& lock,
    const block::bitcoin::Outpoint& id,
    const block::Height height,
    OutputStateMap& from) const noexcept -> bool
{
    auto pSource = from.find(height);

    if (from.end() == pSource) { return true; }

    auto& sourceMap = pSource->second;

    for (auto i{sourceMap.begin()}; i != sourceMap.end();) {
        if (id == *i) {
            i = sourceMap.erase(i);
        } else {
            ++i;
        }
    }

    if (sourceMap.empty()) { from.erase(pSource); }

    return true;
}

auto Wallet::SubchainAddElements(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const ElementMap& elements,
    const VersionNumber version) const noexcept -> bool
{
    Lock lock(lock_);
    subchain_version_[subchain_version_index(balanceNode, subchain, type)] =
        version;
    auto subchainID = subchain_id(balanceNode, subchain, type, version);
    auto newIndices = std::vector<OTIdentifier>{};
    auto highest = Bip32Index{};

    for (const auto& [index, patterns] : elements) {
        auto patternID = pattern_id(subchainID, index);
        auto& vector = patterns_[patternID];
        newIndices.emplace_back(std::move(patternID));
        highest = std::max(highest, index);

        for (const auto& pattern : patterns) {
            vector.emplace_back(index, pattern);
        }
    }

    subchain_last_indexed_[subchainID] = highest;
    auto& index = subchain_pattern_index_[subchainID];

    for (auto& id : newIndices) { index.emplace(std::move(id)); }

    return true;
}

auto Wallet::SubchainDropIndex(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> bool
{
    Lock lock(lock_);
    const auto subchainID = subchain_id(balanceNode, subchain, type, version);

    try {
        for (const auto& patternID : subchain_pattern_index_.at(subchainID)) {
            patterns_.erase(patternID);

            for (auto& [block, set] : match_index_) { set.erase(patternID); }
        }
    } catch (...) {
    }

    subchain_pattern_index_.erase(subchainID);
    subchain_last_indexed_.erase(subchainID);
    subchain_version_.erase(
        subchain_version_index(balanceNode, subchain, type));

    return true;
}

auto Wallet::SubchainIndexVersion(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> VersionNumber
{
    Lock lock(lock_);
    const auto id = subchain_version_index(balanceNode, subchain, type);

    try {

        return subchain_version_.at(id);
    } catch (...) {

        return 0;
    }
}

auto Wallet::SubchainLastIndexed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> std::optional<Bip32Index>
{
    Lock lock(lock_);
    const auto subchainID = subchain_id(balanceNode, subchain, type, version);

    try {
        return subchain_last_indexed_.at(subchainID);
    } catch (...) {
        return {};
    }
}

auto Wallet::SubchainLastProcessed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> block::Position
{
    Lock lock(lock_);

    try {
        return subchain_last_processed_.at(
            subchain_version_index(balanceNode, subchain, type));
    } catch (...) {
        return make_blank<block::Position>::value(api_);
    }
}

auto Wallet::SubchainLastScanned(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> block::Position
{
    Lock lock(lock_);

    try {
        return subchain_last_scanned_.at(
            subchain_version_index(balanceNode, subchain, type));
    } catch (...) {
        return make_blank<block::Position>::value(api_);
    }
}

auto Wallet::SubchainSetLastProcessed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const block::Position& position) const noexcept -> bool
{
    Lock lock(lock_);
    auto& map = subchain_last_processed_;
    auto id = subchain_version_index(balanceNode, subchain, type);
    auto it = map.find(id);

    if (map.end() == it) {
        map.emplace(std::move(id), position);

        return true;
    } else {
        it->second = position;

        return true;
    }
}

auto Wallet::SubchainMatchBlock(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const MatchingIndices& indices,
    const ReadView blockID,
    const VersionNumber version) const noexcept -> bool
{
    Lock lock(lock_);
    auto& matchSet = match_index_[api_.Factory().Data(blockID)];

    for (const auto& index : indices) {
        matchSet.emplace(pattern_id(
            subchain_id(balanceNode, subchain, type, version), index));
    }

    return true;
}

auto Wallet::SubchainSetLastScanned(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const block::Position& position) const noexcept -> bool
{
    Lock lock(lock_);
    auto& map = subchain_last_scanned_;
    auto id = subchain_version_index(balanceNode, subchain, type);
    auto it = map.find(id);

    if (map.end() == it) {
        map.emplace(std::move(id), position);

        return true;
    } else {
        it->second = position;

        return true;
    }
}

auto Wallet::subchain_version_index(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> pSubchainID
{
    auto preimage = OTData{balanceNode};
    preimage->Concatenate(&subchain, sizeof(subchain));
    preimage->Concatenate(&type, sizeof(type));
    auto output = api_.Factory().Identifier();
    output->CalculateDigest(preimage->Bytes());

    return output;
}

auto Wallet::subchain_id(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> pSubchainID
{
    auto preimage = OTData{balanceNode};
    preimage->Concatenate(&subchain, sizeof(subchain));
    preimage->Concatenate(&type, sizeof(type));
    preimage->Concatenate(&version, sizeof(version));
    auto output = api_.Factory().Identifier();
    output->CalculateDigest(preimage->Bytes());

    return output;
}
}  // namespace opentxs::blockchain::database
