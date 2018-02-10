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

#ifndef OPENTXS_CORE_CRYPTO_OTCALLBACK_HPP
#define OPENTXS_CORE_CRYPTO_OTCALLBACK_HPP

#include "opentxs/Forward.hpp"

namespace opentxs
{

class OTPassword;

class OTCallback
{
public:
    OTCallback() {}
    EXPORT virtual ~OTCallback();

    // Asks for password once. (For authentication when using nym.)
    EXPORT virtual void runOne(const char* szDisplay, OTPassword& theOutput)
        const = 0;

    // Asks for password twice. (For confirmation when changing password or
    // creating nym.)
    EXPORT virtual void runTwo(const char* szDisplay, OTPassword& theOutput)
        const = 0;
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_OTCALLBACK_HPP
