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

#include "opentxs/core/contract/peer/ConnectionRequest.hpp"

#include "opentxs/core/String.hpp"

namespace opentxs
{
ConnectionRequest::ConnectionRequest(
    const ConstNym& nym,
    const proto::PeerRequest& serialized)
      : ot_super(nym, serialized)
      , connection_type_(serialized.connectioninfo().type())
{
}

ConnectionRequest::ConnectionRequest(
    const ConstNym& nym,
    const Identifier& recipientID,
    const proto::ConnectionInfoType type)
      : ot_super(nym, recipientID, proto::PEERREQUEST_CONNECTIONINFO)
      , connection_type_(type)
{
}

proto::PeerRequest ConnectionRequest::IDVersion() const
{
    auto contract = ot_super::IDVersion();

    auto& connectioninfo = *contract.mutable_connectioninfo();
    connectioninfo.set_version(version_);
    connectioninfo.set_type(connection_type_);

    return contract;
}
} // namespace opentxs
