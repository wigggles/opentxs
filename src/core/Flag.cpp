// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/Types.hpp"

#include <atomic>

#include "Flag.hpp"

template class opentxs::Pimpl<opentxs::Flag>;

namespace opentxs
{
OTFlag Flag::Factory(const bool state)
{
    return OTFlag(new implementation::Flag(state));
}

namespace implementation
{
Flag::Flag(const bool state)
    : flag_(state)
{
}

bool Flag::Toggle()
{
    auto expected = flag_.load();

    while (false == flag_.compare_exchange_weak(expected, !expected)) { ; }

    return expected;
}
}  // namespace implementation
}  // namespace opentxs
