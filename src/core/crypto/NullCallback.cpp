// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTCallback.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"

#include "NullCallback.hpp"

#define OPENTXS_NULL_PASSWORD "opentxs"

namespace opentxs
{
OTCallback* Factory::NullCallback()
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
