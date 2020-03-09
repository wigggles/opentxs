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

        auto transactions =
            std::vector<std::pair<bb::EncodedTransaction, Space>>{};

        while (transactions.size() < transactionCount) {
            auto& data = transactions.emplace_back();
            auto& [transaction, txid] = data;
            auto& [version, inputs, outputs, locktime] = transaction;
            auto inCount = std::size_t{0};
            auto outCount = std::size_t{0};
            auto start{it};
            auto end{start};
            expectedSize += sizeof(version);

            if (in.size() < expectedSize) {
                throw std::runtime_error("Partial transaction (version)");
            }

            std::memcpy(static_cast<void*>(&version), it, sizeof(version));
            std::advance(it, sizeof(version));
            expectedSize += 1;

            if (in.size() < expectedSize) {
                throw std::runtime_error("Partial transaction (txin count)");
            }

            if (false == bb::DecodeCompactSizeFromPayload(
                             it, expectedSize, in.size(), inCount)) {
                throw std::runtime_error("Failed to decode txin count");
            }

            if (0 == inCount) {
                throw std::runtime_error("Invalid transaction (no inputs)");
            }

            while (inputs.size() < inCount) {
                auto& input = inputs.emplace_back();
                auto& [outpoint, script, sequence] = input;
                auto scriptBytes = std::size_t{0};
                expectedSize += sizeof(outpoint);

                if (in.size() < expectedSize) {
                    throw std::runtime_error("Partial input (outpoint)");
                }

                std::memcpy(
                    static_cast<void*>(&outpoint), it, sizeof(outpoint));
                std::advance(it, sizeof(outpoint));
                expectedSize += 1;

                if (in.size() < expectedSize) {
                    throw std::runtime_error("Partial input (script size)");
                }

                if (false == bb::DecodeCompactSizeFromPayload(
                                 it, expectedSize, in.size(), scriptBytes)) {
                    throw std::runtime_error(
                        "Failed to decode input script bytes");
                }

                script.assign(it, it + scriptBytes);
                std::advance(it, scriptBytes);
                expectedSize += sizeof(sequence);

                if (in.size() < expectedSize) {
                    throw std::runtime_error("Partial input (sequence)");
                }

                std::memcpy(
                    static_cast<void*>(&sequence), it, sizeof(sequence));
                std::advance(it, sizeof(sequence));
            }

            expectedSize += 1;

            if (in.size() < expectedSize) {
                throw std::runtime_error("Partial transaction (txout count)");
            }

            if (false == bb::DecodeCompactSizeFromPayload(
                             it, expectedSize, in.size(), outCount)) {
                throw std::runtime_error("Failed to decode txout count");
            }

            if (0 == outCount) {
                throw std::runtime_error("Invalid transaction (no outputs)");
            }

            while (outputs.size() < outCount) {
                auto& output = outputs.emplace_back();
                auto& [value, script] = output;
                auto scriptBytes = std::size_t{0};
                expectedSize += sizeof(value);

                if (in.size() < expectedSize) {
                    throw std::runtime_error("Partial output (value)");
                }

                std::memcpy(static_cast<void*>(&value), it, sizeof(value));
                std::advance(it, sizeof(value));
                expectedSize += 1;

                if (in.size() < expectedSize) {
                    throw std::runtime_error("Partial output (script size)");
                }

                if (false == bb::DecodeCompactSizeFromPayload(
                                 it, expectedSize, in.size(), scriptBytes)) {
                    throw std::runtime_error(
                        "Failed to decode output script bytes");
                }

                script.assign(it, it + scriptBytes);
                std::advance(it, scriptBytes);
            }

            expectedSize += sizeof(locktime);

            if (in.size() < expectedSize) {
                throw std::runtime_error("Partial transaction (lock time)");
            }

            std::memcpy(static_cast<void*>(&locktime), it, sizeof(locktime));
            std::advance(it, sizeof(locktime));
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
                    for (const auto& [outpoint, script, sequence] :
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
            for (const auto& [value, script] : transaction.outputs_) {
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
