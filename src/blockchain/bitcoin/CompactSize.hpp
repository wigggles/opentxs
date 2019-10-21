// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include <cstdint>
#include <stdexcept>

namespace opentxs::blockchain::bitcoin
{
class CompactSize
{
public:
    using Bytes = std::vector<std::byte>;

    // Returns the number of bytes SUBSEQUENT to first_byte
    // Possible output values are: 0, 2, 4, 8
    static std::uint64_t CalculateSize(const std::byte first) noexcept;

    Bytes Encode() const noexcept;
    std::uint64_t Value() const noexcept;

    // Initial marker byte should be omitted
    // Valid inputs are 0, 2, 4, or 8 bytes
    bool Decode(const Bytes& bytes) noexcept;

    CompactSize() noexcept = default;
    explicit CompactSize(std::uint64_t value) noexcept;
    // Initial marker byte should be omitted
    // Valid inputs are 1, 2, 4, or 8 bytes
    // Throws std::invalid_argument for invalid input
    CompactSize(const Bytes& bytes) noexcept(false);
    CompactSize(const CompactSize&) noexcept = default;
    CompactSize(CompactSize&&) noexcept = default;
    CompactSize& operator=(const CompactSize&) noexcept = default;
    CompactSize& operator=(CompactSize&&) noexcept = default;
    CompactSize& operator=(const std::uint64_t rhs) noexcept;

    ~CompactSize() = default;

private:
    std::uint64_t data_;

    template <typename SizeType>
    void convert_to_raw(Bytes& output) const noexcept;
    template <typename SizeType>
    void convert_from_raw(const Bytes& bytes) noexcept;
};
}  // namespace opentxs::blockchain::bitcoin
