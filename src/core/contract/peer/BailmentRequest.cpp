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

#include "opentxs/core/contract/peer/BailmentRequest.hpp"

#include "opentxs/core/String.hpp"

namespace opentxs
{
BailmentRequest::BailmentRequest(
    const ConstNym& nym,
    const proto::PeerRequest& serialized)
      : ot_super(nym, serialized)
      , unit_(serialized.bailment().unitid())
      , server_(serialized.bailment().serverid())
{
}

BailmentRequest::BailmentRequest(
    const ConstNym& nym,
    const Identifier& recipientID,
    const Identifier& unitID,
    const Identifier& serverID)
      : ot_super(nym, recipientID, serverID, proto::PEERREQUEST_BAILMENT)
      , unit_(unitID)
      , server_(serverID)
{
}

proto::PeerRequest BailmentRequest::IDVersion(const Lock& lock) const
{
    auto contract = ot_super::IDVersion(lock);

    auto& bailment = *contract.mutable_bailment();
    bailment.set_version(version_);
    bailment.set_unitid(String(unit_).Get());
    bailment.set_serverid(String(server_).Get());

    return contract;
}
} // namespace opentxs
