// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "blockchain/bitcoin/CompactSize.hpp"

#include <boost/endian/buffers.hpp>

#include <array>

namespace be = boost::endian;

namespace opentxs::blockchain::bitcoin
{
/// output: the decoded value
///
/// input: gets incremented to the byte past the CompactSize
///
/// expectedSize: gets incremented by csBytes
///
/// csExtraBytes: additional bytes consumed by the CompactSize beyond the
/// minimum of 1 byte. For small encoded sizes, this will be 0.
///
/// A CompactSize is at least one byte, so before you attempt to execute this
/// this function position input to the first byte and set expectedSize
/// appropriately to make sure the input contains at least one byte.
/// If the CompactSize is larger than 1 byte, then expectedSize will be updated.
///
/// In all cases input will be advanced as needed.
///
/// Normally you don't need to worry about how many bytes the CompactSize
/// actually occupied because you're just going to move on to reading the next
/// element, but just in case you do there's an overload which outputs
/// csExtraBytes
bool DecodeCompactSizeFromPayload(
    const std::byte*& input,
    std::size_t& expectedSize,
    const std::size_t size,
    std::size_t& output) noexcept;
bool DecodeCompactSizeFromPayload(
    const std::byte*& input,
    std::size_t& expectedSize,
    const std::size_t size,
    CompactSize& output) noexcept;
bool DecodeCompactSizeFromPayload(
    const std::byte*& input,
    std::size_t& expectedSize,
    const std::size_t size,
    std::size_t& output,
    std::size_t& csExtraBytes) noexcept;

struct EncodedOutpoint {
    std::array<std::byte, 32> txid_{};
    be::little_uint32_buf_t index_{};
};

struct EncodedInput {
    EncodedOutpoint outpoint_{};
    CompactSize cs_{};
    Space script_{};
    be::little_uint32_buf_t sequence_{};

    auto size() const noexcept -> std::size_t;
};

struct EncodedOutput {
    be::little_int64_buf_t value_{};
    CompactSize cs_{};
    Space script_{};

    auto size() const noexcept -> std::size_t;
};

struct EncodedTransaction {
    be::little_int32_buf_t version_{};
    CompactSize input_count_{};
    std::vector<EncodedInput> inputs_{};
    CompactSize output_count_{};
    std::vector<EncodedOutput> outputs_{};
    be::little_uint32_buf_t lock_time_{};

    OPENTXS_EXPORT static auto Deserialize(const ReadView bytes) noexcept(false)
        -> EncodedTransaction;

    auto size() const noexcept -> std::size_t;
};
}  // namespace opentxs::blockchain::bitcoin
