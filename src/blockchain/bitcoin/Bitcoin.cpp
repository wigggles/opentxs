// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "internal/blockchain/bitcoin/Bitcoin.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <cstring>
#include <iterator>
#include <numeric>
#include <stdexcept>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api
}  // namespace opentxs

namespace bb = opentxs::blockchain::bitcoin;

namespace opentxs::blockchain::bitcoin
{
const auto cb = [](const auto lhs, const auto& in) -> std::size_t {
    return lhs + in.size();
};

auto HasSegwit(
    ByteIterator& input,
    std::size_t& expectedSize,
    const std::size_t size) noexcept(false) -> std::optional<std::byte>
{
    auto output = std::optional<std::byte>{};

    OT_ASSERT(false == output.has_value());

    if (size < (expectedSize + 2)) {
        throw std::runtime_error("Unable to check for segwit marker bytes");
    }

    if (nullptr == input) {
        throw std::runtime_error("Invalid input iterator");
    }

    static const auto blank = std::byte{0x00};

    if (blank != *input) {
        LogInsane(__FUNCTION__)(": No marker byte").Flush();

        return output;
    }

    auto flag = *(input + 1);

    if (blank == flag) {
        LogInsane(__FUNCTION__)(": No flag byte").Flush();

        return output;
    }

    output = flag;
    std::advance(input, 2);
    expectedSize += 2;

    OT_ASSERT(output.has_value());

    return output;
}

auto EncodedInput::size() const noexcept -> std::size_t
{
    return sizeof(outpoint_) + cs_.Total() + sizeof(sequence_);
}

auto EncodedInputWitness::size() const noexcept -> std::size_t
{
    return cs_.Size() +
           std::accumulate(std::begin(items_), std::end(items_), 0u, cb);
}

auto EncodedOutput::size() const noexcept -> std::size_t
{
    return sizeof(value_) + cs_.Total();
}

auto EncodedTransaction::Deserialize(
    const api::Core& api,
    const blockchain::Type chain,
    const ReadView in) noexcept(false) -> EncodedTransaction
{
    if ((nullptr == in.data()) || (0 == in.size())) {
        throw std::runtime_error("Invalid bytes");
    }

    auto output = EncodedTransaction{};
    auto& [version, segwit, inCount, inputs, outCount, outputs, witnesses, locktime, wtxid, txid] =
        output;
    auto it = reinterpret_cast<ByteIterator>(in.data());
    const auto start{it};
    auto expectedSize = sizeof(version);

    if (in.size() < expectedSize) {
        throw std::runtime_error("Partial transaction (version)");
    }

    std::memcpy(static_cast<void*>(&version), it, sizeof(version));
    std::advance(it, sizeof(version));
    LogTrace("opentxs::blockchain::bitcoin::EncodedTransaction::")(
        __FUNCTION__)(": Tx version: ")(version.value())
        .Flush();
    segwit = HasSegwit(it, expectedSize, in.size());

    if (segwit.has_value()) {
        LogTrace("opentxs::blockchain::bitcoin::EncodedTransaction::")(
            __FUNCTION__)(": Found segwit transaction flag: ")(
            std::to_integer<std::uint8_t>(segwit.value()))
            .Flush();
    } else {
        LogTrace("opentxs::blockchain::bitcoin::EncodedTransaction::")(
            __FUNCTION__)(": Found non-segwit transaction")
            .Flush();
    }

    expectedSize += 1;

    if (in.size() < expectedSize) {
        throw std::runtime_error("Partial transaction (txin count)");
    }

    if (false == bb::DecodeCompactSizeFromPayload(
                     it, expectedSize, in.size(), inCount)) {
        throw std::runtime_error("Failed to decode txin count");
    }

    LogTrace("opentxs::blockchain::bitcoin::EncodedTransaction::")(
        __FUNCTION__)(": Tx input count: ")(inCount.Value())
        .Flush();

    while (inputs.size() < inCount.Value()) {
        auto& input = inputs.emplace_back();
        auto& [outpoint, scriptBytes, script, sequence] = input;
        expectedSize += sizeof(outpoint);

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial input (outpoint)");
        }

        std::memcpy(static_cast<void*>(&outpoint), it, sizeof(outpoint));
        std::advance(it, sizeof(outpoint));
        expectedSize += 1;

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial input (script size)");
        }

        if (false == bb::DecodeCompactSizeFromPayload(
                         it, expectedSize, in.size(), scriptBytes)) {
            throw std::runtime_error("Failed to decode input script bytes");
        }

        LogTrace("opentxs::blockchain::bitcoin::EncodedTransaction::")(
            __FUNCTION__)(": input script bytes: ")(scriptBytes.Value())
            .Flush();
        expectedSize += scriptBytes.Value();

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial input (script)");
        }

        script.assign(it, it + scriptBytes.Value());
        std::advance(it, scriptBytes.Value());
        expectedSize += sizeof(sequence);

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial input (sequence)");
        }

        std::memcpy(static_cast<void*>(&sequence), it, sizeof(sequence));
        std::advance(it, sizeof(sequence));
        LogTrace("opentxs::blockchain::bitcoin::EncodedTransaction::")(
            __FUNCTION__)(": sequence: ")(sequence.value())
            .Flush();
    }

    expectedSize += 1;

    if (in.size() < expectedSize) {
        throw std::runtime_error("Partial transaction (txout count)");
    }

    if (false == bb::DecodeCompactSizeFromPayload(
                     it, expectedSize, in.size(), outCount)) {
        throw std::runtime_error("Failed to decode txout count");
    }

    LogTrace("opentxs::blockchain::bitcoin::EncodedTransaction::")(
        __FUNCTION__)(": Tx output count: ")(outCount.Value())
        .Flush();

    while (outputs.size() < outCount.Value()) {
        auto& output = outputs.emplace_back();
        auto& [value, scriptBytes, script] = output;
        expectedSize += sizeof(value);

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial output (value)");
        }

        std::memcpy(static_cast<void*>(&value), it, sizeof(value));
        std::advance(it, sizeof(value));
        LogTrace("opentxs::blockchain::bitcoin::EncodedTransaction::")(
            __FUNCTION__)(": value: ")(value.value())
            .Flush();
        expectedSize += 1;

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial output (script size)");
        }

        if (false == bb::DecodeCompactSizeFromPayload(
                         it, expectedSize, in.size(), scriptBytes)) {
            throw std::runtime_error("Failed to decode output script bytes");
        }

        LogTrace("opentxs::blockchain::bitcoin::EncodedTransaction::")(
            __FUNCTION__)(": output script bytes: ")(scriptBytes.Value())
            .Flush();
        expectedSize += scriptBytes.Value();

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial output (script)");
        }

        script.assign(it, it + scriptBytes.Value());
        std::advance(it, scriptBytes.Value());
    }

    if (segwit.has_value()) {
        for (auto i{0u}; i < inputs.size(); ++i) {
            auto& [witnessCount, witnessItems] = witnesses.emplace_back();

            if (false == bb::DecodeCompactSizeFromPayload(
                             it, expectedSize, in.size(), witnessCount)) {
                throw std::runtime_error("Failed to witness item count");
            }

            LogTrace("opentxs::blockchain::bitcoin::EncodedTransaction::")(
                __FUNCTION__)(": witness ")(i)(" contains ")(
                witnessCount.Value())(" pushes")
                .Flush();

            for (auto w{0u}; w < witnessCount.Value(); ++w) {
                auto& [witnessBytes, push] = witnessItems.emplace_back();

                if (false == bb::DecodeCompactSizeFromPayload(
                                 it, expectedSize, in.size(), witnessBytes)) {
                    throw std::runtime_error("Failed to witness item bytes");
                }

                LogTrace("opentxs::blockchain::bitcoin::EncodedTransaction::")(
                    __FUNCTION__)(": push ")(w)(" bytes: ")(
                    witnessBytes.Value())
                    .Flush();
                expectedSize += witnessBytes.Value();

                if (in.size() < expectedSize) {
                    throw std::runtime_error("Partial witness item");
                }

                push.assign(it, it + witnessBytes.Value());
                std::advance(it, witnessBytes.Value());
            }
        }
    }

    if ((0 != witnesses.size()) && (witnesses.size() != inputs.size())) {
        throw std::runtime_error("Invalid witnesses count");
    }

    expectedSize += sizeof(locktime);

    if (in.size() < expectedSize) {
        throw std::runtime_error("Partial transaction (lock time)");
    }

    std::memcpy(static_cast<void*>(&locktime), it, sizeof(locktime));
    std::advance(it, sizeof(locktime));
    const auto txBytes = static_cast<std::size_t>(std::distance(start, it));
    LogTrace("opentxs::blockchain::bitcoin::EncodedTransaction::")(
        __FUNCTION__)(": lock time: ")(locktime.value())
        .Flush();
    const auto gotWtxid = TransactionHash(
        api, chain, ReadView{in.data(), txBytes}, writer(wtxid));

    if (false == gotWtxid) {
        throw std::runtime_error("Failed to calculate txid / wtxid");
    }

    if (segwit.has_value()) {
        const auto gotTxid = TransactionHash(
            api, chain, reader(output.txid_preimage()), writer(txid));

        if (false == gotTxid) {
            throw std::runtime_error("Failed to calculate txid");
        }
    } else {
        txid = wtxid;
    }

    OT_ASSERT(txBytes == output.size());

    return output;
}

auto EncodedTransaction::txid_preimage() const noexcept -> Space
{
    auto output = space(txid_size());
    auto it = reinterpret_cast<std::byte*>(output.data());
    std::memcpy(it, static_cast<const void*>(&version_), sizeof(version_));
    std::advance(it, sizeof(version_));

    {
        const auto bytes = input_count_.Encode();
        std::memcpy(it, bytes.data(), bytes.size());
        std::advance(it, bytes.size());
    }

    for (const auto& [outpoint, cs, script, sequence] : inputs_) {
        const auto bytes = cs.Encode();
        std::memcpy(it, static_cast<const void*>(&outpoint), sizeof(outpoint));
        std::advance(it, sizeof(outpoint));
        std::memcpy(it, bytes.data(), bytes.size());
        std::advance(it, bytes.size());
        std::memcpy(it, script.data(), script.size());
        std::advance(it, script.size());
        std::memcpy(it, static_cast<const void*>(&sequence), sizeof(sequence));
        std::advance(it, sizeof(sequence));
    }

    {
        const auto bytes = output_count_.Encode();
        std::memcpy(it, bytes.data(), bytes.size());
        std::advance(it, bytes.size());
    }

    for (const auto& [value, cs, script] : outputs_) {
        const auto bytes = cs.Encode();
        std::memcpy(it, static_cast<const void*>(&value), sizeof(value));
        std::advance(it, sizeof(value));
        std::memcpy(it, bytes.data(), bytes.size());
        std::advance(it, bytes.size());
        std::memcpy(it, script.data(), script.size());
        std::advance(it, script.size());
    }

    std::memcpy(it, static_cast<const void*>(&lock_time_), sizeof(lock_time_));
    std::advance(it, sizeof(lock_time_));

    return output;
}

auto EncodedTransaction::txid_size() const noexcept -> std::size_t
{
    return sizeof(version_) + input_count_.Size() +
           std::accumulate(std::begin(inputs_), std::end(inputs_), 0u, cb) +
           output_count_.Size() +
           std::accumulate(std::begin(outputs_), std::end(outputs_), 0u, cb) +
           sizeof(lock_time_);
}

auto EncodedTransaction::size() const noexcept -> std::size_t
{
    return txid_size() +
           (segwit_flag_.has_value()
                ? std::accumulate(
                      std::begin(witnesses_), std::end(witnesses_), 2u, cb)
                : std::size_t{0});
}

auto EncodedWitnessItem::size() const noexcept -> std::size_t
{
    return cs_.Total();
}
}  // namespace opentxs::blockchain::bitcoin
