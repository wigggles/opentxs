// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "core/crypto/NullCallback.hpp"  // IWYU pragma: associated

#include <memory>

#include "Factory.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"

#define OPENTXS_NULL_PASSWORD "opentxs"

namespace opentxs
{
auto Factory::NullCallback() -> OTCallback*
{
    return new implementation::NullCallback();
}
}  // namespace opentxs

namespace opentxs::implementation
{
const std::string NullCallback::password_{OPENTXS_NULL_PASSWORD};

void NullCallback::runOne(const char*, OTPassword& output) const
{
    output.setPassword(password_);
}

void NullCallback::runTwo(const char* display, OTPassword& output) const
{
    runOne(display, output);
}
}  // namespace opentxs::implementation
