// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "blockchain/bitcoin/CompactSize.hpp"

namespace opentxs::blockchain::bitcoin
{
bool DecodeCompactSizeFromPayload(
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    std::size_t& output) noexcept;
bool DecodeCompactSizeFromPayload(
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    std::size_t& output,
    std::size_t& csBytes) noexcept;
}  // namespace opentxs::blockchain::bitcoin
