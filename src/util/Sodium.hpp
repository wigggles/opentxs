// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::crypto::sodium
{
auto ExpandSeed(
    const OTPassword& seed,
    OTPassword& privateKey,
    Data& publicKey) noexcept -> bool;
}  // namespace opentxs::crypto::sodium
