// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::blockchain::transaction::bitcoin
{

class Witness
{
public:
    static bool DecodeFromPayload(
        const std::byte*& it,
        std::size_t& expectedSize,
        const std::size_t size,
        Witness& txWitness) noexcept;

    const std::vector<OTData>& components() const { return components_; }
    OTData Encode() const noexcept;

    Witness(const std::vector<OTData>& components) noexcept;
    Witness() noexcept;

private:
    std::vector<OTData> components_;
};
}  // namespace opentxs::blockchain::transaction::bitcoin
