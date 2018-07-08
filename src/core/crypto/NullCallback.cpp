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

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTCallback.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"

#include "NullCallback.hpp"

#define OPENTXS_NULL_PASSWORD "test"

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
