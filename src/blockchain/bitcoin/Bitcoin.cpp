// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include <numeric>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"

namespace bb = opentxs::blockchain::bitcoin;

namespace opentxs::blockchain::bitcoin
{
auto EncodedInput::size() const noexcept -> std::size_t
{
    return sizeof(outpoint_) + cs_.Total() + sizeof(sequence_);
}

auto EncodedOutput::size() const noexcept -> std::size_t
{
    return sizeof(value_) + cs_.Total();
}

auto EncodedTransaction::Deserialize(const ReadView in) noexcept(false)
    -> EncodedTransaction
{
    if ((nullptr == in.data()) || (0 == in.size())) {
        throw std::runtime_error("Invalid bytes");
    }

    auto output = EncodedTransaction{};
    auto& [version, inCount, inputs, outCount, outputs, locktime] = output;
    auto it = reinterpret_cast<const std::byte*>(in.data());
    auto expectedSize = sizeof(version);

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

    if (0 == inCount.Value()) {
        throw std::runtime_error("Invalid transaction (no inputs)");
    }

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
    }

    expectedSize += 1;

    if (in.size() < expectedSize) {
        throw std::runtime_error("Partial transaction (txout count)");
    }

    if (false == bb::DecodeCompactSizeFromPayload(
                     it, expectedSize, in.size(), outCount)) {
        throw std::runtime_error("Failed to decode txout count");
    }

    if (0 == outCount.Value()) {
        throw std::runtime_error("Invalid transaction (no outputs)");
    }

    while (outputs.size() < outCount.Value()) {
        auto& output = outputs.emplace_back();
        auto& [value, scriptBytes, script] = output;
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
            throw std::runtime_error("Failed to decode output script bytes");
        }

        expectedSize += scriptBytes.Value();

        if (in.size() < expectedSize) {
            throw std::runtime_error("Partial output (script)");
        }

        script.assign(it, it + scriptBytes.Value());
        std::advance(it, scriptBytes.Value());
    }

    expectedSize += sizeof(locktime);

    if (in.size() < expectedSize) {
        throw std::runtime_error("Partial transaction (lock time)");
    }

    std::memcpy(static_cast<void*>(&locktime), it, sizeof(locktime));
    std::advance(it, sizeof(locktime));

    return output;
}

auto EncodedTransaction::size() const noexcept -> std::size_t
{
    const auto cb = [](const auto lhs, const auto& in) -> std::size_t {
        return lhs + in.size();
    };

    return sizeof(version_) + input_count_.Size() +
           std::accumulate(std::begin(inputs_), std::end(inputs_), 0u, cb) +
           output_count_.Size() +
           std::accumulate(std::begin(outputs_), std::end(outputs_), 0u, cb) +
           sizeof(lock_time_);
}
}  // namespace opentxs::blockchain::bitcoin
