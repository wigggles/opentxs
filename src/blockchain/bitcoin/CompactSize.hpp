// Copyright (c) 2010-2020 The Open-Transactions developers
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
    OPENTXS_EXPORT static std::uint64_t CalculateSize(
        const std::byte first) noexcept;

    OPENTXS_EXPORT Bytes Encode() const noexcept;
    OPENTXS_EXPORT bool Encode(AllocateOutput destination) const noexcept;
    // Number of bytes the CompactSize will occupy
    OPENTXS_EXPORT std::size_t Size() const noexcept;
    // Number of bytes the CompactSize and associated data will occupy
    OPENTXS_EXPORT std::size_t Total() const noexcept;
    OPENTXS_EXPORT std::uint64_t Value() const noexcept;

    // Initial marker byte should be omitted
    // Valid inputs are 0, 2, 4, or 8 bytes
    OPENTXS_EXPORT bool Decode(const Bytes& bytes) noexcept;

    OPENTXS_EXPORT CompactSize() noexcept = default;
    OPENTXS_EXPORT explicit CompactSize(std::uint64_t value) noexcept;
    // Initial marker byte should be omitted
    // Valid inputs are 1, 2, 4, or 8 bytes
    // Throws std::invalid_argument for invalid input
    OPENTXS_EXPORT CompactSize(const Bytes& bytes) noexcept(false);
    OPENTXS_EXPORT CompactSize(const CompactSize&) noexcept = default;
    OPENTXS_EXPORT CompactSize(CompactSize&&) noexcept = default;
    OPENTXS_EXPORT CompactSize& operator=(const CompactSize&) noexcept =
        default;
    OPENTXS_EXPORT CompactSize& operator=(CompactSize&&) noexcept = default;
    OPENTXS_EXPORT CompactSize& operator=(const std::uint64_t rhs) noexcept;

    OPENTXS_EXPORT ~CompactSize() = default;

private:
    std::uint64_t data_;

    template <typename SizeType>
    auto convert_to_raw(AllocateOutput output) const noexcept -> bool;
    template <typename SizeType>
    auto convert_from_raw(const Bytes& bytes) noexcept -> void;
};
}  // namespace opentxs::blockchain::bitcoin
