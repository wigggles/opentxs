// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include "internal/blockchain/bitcoin/Bitcoin.hpp"

#include <boost/endian/arithmetic.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/endian/buffers.hpp>

#include <algorithm>
#include <memory>
#include <cstdint>
#include <cstring>

#include "CompactSize.hpp"

#define OT_COMPACT_SIZE_THRESHOLD_1 252
#define OT_COMPACT_SIZE_THRESHOLD_3 65535
#define OT_COMPACT_SIZE_THRESHOLD_5 4294967295

#define OT_COMPACT_SIZE_PREFIX_3 0xfd
#define OT_COMPACT_SIZE_PREFIX_5 0xfe
#define OT_COMPACT_SIZE_PREFIX_9 0xff

#define OT_METHOD "opentxs::blockchain::bitcoin::CompactSize::"

namespace be = boost::endian;

namespace opentxs::blockchain::bitcoin
{
bool DecodeCompactSizeFromPayload(
    const std::byte*& it,
    std::size_t& expected,
    const std::size_t size,
    std::size_t& output) noexcept
{
    std::size_t notUsed{};

    return DecodeCompactSizeFromPayload(it, expected, size, output, notUsed);
}

bool DecodeCompactSizeFromPayload(
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    std::size_t& output,
    std::size_t& csBytes) noexcept
{
    std::size_t csValue{0};
    output = csValue;  // zero

    if (std::byte{0} == *it) {
        it += 1;  // This compact size contains a zero.
    } else {
        {
            const auto max =
                std::uint64_t{std::numeric_limits<std::size_t>::max()};
            const auto lhs = std::uint64_t{csBytes};
            const auto rhs = CompactSize::CalculateSize(*it);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
            // std::size_t might be 32 bit
            if (max < rhs) {
                LogOutput("opentxs::blockchain::bitcoin::")(__FUNCTION__)(
                    ": Size too big")
                    .Flush();

                return false;
            }
#pragma GCC diagnostic pop

            csBytes = static_cast<std::size_t>(rhs);

            if ((max - lhs) < rhs) {
                LogOutput("opentxs::blockchain::bitcoin::")(__FUNCTION__)(
                    ": overflow")
                    .Flush();

                return false;
            }

            expectedSize += csBytes;
        }

        if (expectedSize > size) { return false; }

        if (0 == csBytes) {
            csValue = std::to_integer<std::uint8_t>(*it);
            it += 1;
        } else {
            it += 1;
            CompactSize::Bytes bytes{it, it + csBytes};
            csValue = CompactSize(bytes).Value();
            it += csBytes;
        }
    }

    output = csValue;

    return true;
}

CompactSize::CompactSize(std::uint64_t value) noexcept
    : data_(value)
{
    static_assert(sizeof(data_) >= sizeof(std::size_t));
}

CompactSize::CompactSize(const Bytes& bytes) noexcept(false)
    : data_(0)
{
    if (false == Decode(bytes)) {
        throw std::invalid_argument(
            "Wrong number of bytes: " + std::to_string(bytes.size()));
    }
}

CompactSize& CompactSize::operator=(const std::uint64_t rhs) noexcept
{
    data_ = rhs;

    return *this;
}

std::uint64_t CompactSize::CalculateSize(const std::byte first) noexcept
{
    auto marker{reinterpret_cast<const uint8_t&>(first)};

    if (OT_COMPACT_SIZE_PREFIX_9 == marker) {
        return 8;
    } else if (OT_COMPACT_SIZE_PREFIX_5 == marker) {
        return 4;
    } else if (OT_COMPACT_SIZE_PREFIX_3 == marker) {
        return 2;
    } else {
        return 0;
    }
}

template <typename SizeType>
void CompactSize::convert_from_raw(const std::vector<std::byte>& bytes) noexcept
{
    SizeType value{0};
    std::memcpy(&value, bytes.data(), sizeof(value));
    be::little_to_native_inplace(value);
    data_ = value;
}

template <typename SizeType>
void CompactSize::convert_to_raw(std::vector<std::byte>& output) const noexcept
{
    OT_ASSERT(std::numeric_limits<SizeType>::max() >= data_);

    auto value{static_cast<SizeType>(data_)};
    be::native_to_little_inplace(value);
    const auto bytes = Data::Factory(&value, sizeof(value));

    OT_ASSERT(sizeof(SizeType) == bytes->size());

    for (const auto& byte : bytes.get()) { output.emplace_back(byte); }
}

bool CompactSize::Decode(const std::vector<std::byte>& bytes) noexcept
{
    bool output{true};

    if (sizeof(std::uint8_t) == bytes.size()) {
        convert_from_raw<std::uint8_t>(bytes);
    } else if (sizeof(std::uint16_t) == bytes.size()) {
        convert_from_raw<std::uint16_t>(bytes);
    } else if (sizeof(std::uint32_t) == bytes.size()) {
        convert_from_raw<std::uint32_t>(bytes);
    } else if (sizeof(std::uint64_t) == bytes.size()) {
        convert_from_raw<std::uint64_t>(bytes);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong number of bytes: ")(
            bytes.size())
            .Flush();
        output = false;
    }

    return output;
}

std::vector<std::byte> CompactSize::Encode() const noexcept
{
    std::vector<std::byte> output{};

    if (data_ <= OT_COMPACT_SIZE_THRESHOLD_1) {
        convert_to_raw<std::uint8_t>(output);
    } else if (data_ <= OT_COMPACT_SIZE_THRESHOLD_3) {
        output.emplace_back(std::byte(OT_COMPACT_SIZE_PREFIX_3));
        convert_to_raw<std::uint16_t>(output);
    } else if (data_ <= OT_COMPACT_SIZE_THRESHOLD_5) {
        output.emplace_back(std::byte(OT_COMPACT_SIZE_PREFIX_5));
        convert_to_raw<std::uint32_t>(output);
    } else {
        output.emplace_back(std::byte(OT_COMPACT_SIZE_PREFIX_9));
        convert_to_raw<std::uint64_t>(output);
    }

    return output;
}

std::uint64_t CompactSize::Value() const noexcept { return data_; }
}  // namespace opentxs::blockchain::bitcoin
