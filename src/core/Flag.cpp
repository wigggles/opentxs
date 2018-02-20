/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/stdafx.hpp"

#include "opentxs/Types.hpp"

#include "Flag.hpp"

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

void Flag::Off() { Set(false); }

void Flag::On() { Set(true); }

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
}  // namespace opentxs::implementation
}  // namespace opentxs
