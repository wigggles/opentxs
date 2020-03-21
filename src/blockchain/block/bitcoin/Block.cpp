// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/core/Log.hpp"

#include "blockchain/block/Block.hpp"
#include "internal/api/Api.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/Block.hpp"

#include <cstring>
#include <stdexcept>

#include "Block.hpp"

//  #define OT_METHOD
//  "opentxs::blockchain::block::bitcoin::implementation::Block::"

namespace opentxs
{
using ReturnType = blockchain::block::bitcoin::implementation::Block;

auto Factory::BitcoinBlock(
    const api::internal::Core& api,
    const blockchain::Type chain,
    const ReadView in) noexcept
    -> std::shared_ptr<blockchain::block::bitcoin::Block>
{
    auto data = api.Factory().Data(in);

    try {
        if ((nullptr == in.data()) || (0 == in.size())) {
            throw std::runtime_error("Invalid block input");
        }

        auto it = reinterpret_cast<const std::byte*>(in.data());
        auto expectedSize = std::size_t{80};

        if (in.size() < expectedSize) {
            throw std::runtime_error("Block size too short (header)");
        }

        auto pHeader =
            BitcoinBlockHeader(api, {reinterpret_cast<const char*>(it), 80});

        if (false == bool(pHeader)) {
            throw std::runtime_error("Invalid block header");
        }

        std::advance(it, 80);
        expectedSize += 1;

        if (in.size() < expectedSize) {
            throw std::runtime_error(
                "Block size too short (transaction count)");
        }

        auto transactionCount = std::size_t{};

        if (false == bb::DecodeCompactSizeFromPayload(
                         it, expectedSize, in.size(), transactionCount)) {
            throw std::runtime_error("Failed to decode transaction count");
        }

        if (0 == transactionCount) { throw std::runtime_error("Empty block"); }

        using Encoded = std::pair<bb::EncodedTransaction, Space>;
        auto transactions = std::vector<Encoded>{};

        while (transactions.size() < transactionCount) {
            auto start{it};
            auto end{start};
            auto& data = transactions.emplace_back(
                Encoded{bb::EncodedTransaction::Deserialize(
                            ReadView{reinterpret_cast<const char*>(it),
                                     in.size() - expectedSize}),
                        Space{}});
            auto& [transaction, txid] = data;
            const auto txBytes = transaction.size();
            std::advance(it, txBytes);
            expectedSize += txBytes;
            end = it;
            const auto gotTxid = api.Crypto().Hash().Digest(
                proto::HASHTYPE_SHA256D,  // TODO stop hardcoding
                ReadView{reinterpret_cast<const char*>(start),
                         static_cast<std::size_t>(std::distance(start, end))},
                writer(txid));

            if (false == gotTxid) {
                throw std::runtime_error("Failed to calculate txid");
            }
        }

        return std::make_shared<ReturnType>(
            api, chain, std::move(pHeader), std::move(transactions));
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
Block::Block(
    const api::internal::Core& api,
    const blockchain::Type chain,
    std::unique_ptr<const internal::Header> header,
    std::vector<std::pair<bb::EncodedTransaction, Space>>&&
        parsed) noexcept(false)
    : block::implementation::Block(api, *header)
    , header_p_(std::move(header))
    , transactions_(std::move(parsed))
    , header_(*header_p_)
{
    if (false == bool(header_p_)) {
        throw std::runtime_error("Invalid header");
    }
}

auto Block::FindMatches(
    [[maybe_unused]] const FilterType type,
    const Patterns& outpoints,
    const Patterns& scripts) const noexcept -> Matches
{
    if (0 == (outpoints.size() + scripts.size())) { return {}; }

    auto output = Matches{};

    for (const auto& [transaction, txid] : transactions_) {
        if (0 < outpoints.size()) {
            for (const auto& [id, element] : outpoints) {
                // TODO cache this before starting the loops
                auto eOutpoint = bb::EncodedOutpoint{};

                if (sizeof(element) != element.size()) { continue; }

                std::memcpy(
                    static_cast<void*>(&eOutpoint),
                    element.data(),
                    element.size());
                // TODO cache this before starting the loops
                const auto& [eTxid, eIndex] = eOutpoint;
                const auto txidMatch =
                    (eTxid.size() == txid.size()) &&
                    (0 == std::memcmp(eTxid.data(), txid.data(), eTxid.size()));

                if (txidMatch) {
                    if (eIndex.value() < transaction.outputs_.size()) {
                        output.emplace_back(api_.Factory().Data(txid), id);
                    }
                } else {
                    for (const auto& [outpoint, cs, script, sequence] :
                         transaction.inputs_) {
                        if (element.size() != sizeof(outpoint)) { continue; }

                        const auto match =
                            (0 ==
                             std::memcmp(
                                 element.data(), &outpoint, element.size()));

                        if (match) {
                            output.emplace_back(api_.Factory().Data(txid), id);
                        }
                    }
                }
            }
        }

        if (0 < scripts.size()) {
            for (const auto& [value, cs, script] : transaction.outputs_) {
                for (const auto& [id, element] : scripts) {
                    if (element.size() != script.size()) { continue; }

                    const auto match =
                        (0 ==
                         std::memcmp(
                             element.data(), script.data(), element.size()));

                    if (match) {
                        output.emplace_back(api_.Factory().Data(txid), id);
                    }
                }
            }
        }
    }

    return output;
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
