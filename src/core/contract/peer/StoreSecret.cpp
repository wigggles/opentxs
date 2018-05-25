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

#include "opentxs/core/contract/peer/StoreSecret.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"

#define CURRENT_VERSION 4

namespace opentxs
{
StoreSecret::StoreSecret(
    const ConstNym& nym,
    const proto::PeerRequest& serialized)
    : ot_super(nym, serialized)
    , secret_type_(serialized.storesecret().type())
    , primary_(serialized.storesecret().primary())
    , secondary_(serialized.storesecret().secondary())
{
}

StoreSecret::StoreSecret(
    const ConstNym& nym,
    const Identifier& recipientID,
    const proto::SecretType type,
    const std::string& primary,
    const std::string& secondary,
    const Identifier& serverID)
    : ot_super(
          nym,
          CURRENT_VERSION,
          recipientID,
          serverID,
          proto::PEERREQUEST_STORESECRET)
    , secret_type_(type)
    , primary_(primary)
    , secondary_(secondary)
{
}

proto::PeerRequest StoreSecret::IDVersion(const Lock& lock) const
{
    auto contract = ot_super::IDVersion(lock);

    auto& storesecret = *contract.mutable_storesecret();
    storesecret.set_version(version_);
    storesecret.set_type(secret_type_);
    storesecret.set_primary(primary_);
    storesecret.set_secondary(secondary_);

    return contract;
}
}  // namespace opentxs
