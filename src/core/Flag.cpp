// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
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

Flag::operator bool() const
{
#if OT_VALGRIND
    Lock lock(lock_);
#endif

    return flag_.load();
}

Flag* Flag::clone() const { return new Flag(flag_.load()); }

bool Flag::Off() { return Set(false); }

bool Flag::On() { return !Set(true); }

bool Flag::Set(const bool value)
{
#if OT_VALGRIND
    Lock lock(lock_);
#endif
    return flag_.exchange(value);
}

bool Flag::Toggle()
{
    Lock lock(lock_);
    const auto value{flag_.load()};

    return flag_.exchange(!value);
}
}  // namespace implementation
}  // namespace opentxs
