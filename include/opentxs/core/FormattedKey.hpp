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

#ifndef OPENTXS_CORE_FORMATTEDKEY_HPP
#define OPENTXS_CORE_FORMATTEDKEY_HPP

#include <opentxs/core/String.hpp>

namespace opentxs
{

/// This class is identical to opentxs::String.
///
/// The reason it exists is to allow crypto key implementations
/// which have more than one way to represent a key as a String
/// to use function overloading on key handling functions which
/// must act differently based on which representation is being
/// used.
class FormattedKey : public String
{
typedef String ot_super;

public:
    FormattedKey();
    EXPORT FormattedKey(const std::string& value);
};

} // namespace opentxs

#endif // OPENTXS_CORE_FORMATTEDKEY_HPP
