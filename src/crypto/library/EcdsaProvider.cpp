// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "EcdsaProvider.hpp"

// #define OT_METHOD "opentxs::crypto::implementation::EcdsaProvider::"

namespace opentxs::crypto::implementation
{
EcdsaProvider::EcdsaProvider(const api::Crypto& crypto)
    : crypto_(crypto)
{
}
}  // namespace opentxs::crypto::implementation
