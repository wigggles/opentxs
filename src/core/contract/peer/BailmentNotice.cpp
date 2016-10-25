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

#include "opentxs/core/contract/peer/BailmentNotice.hpp"

#include "opentxs/core/String.hpp"

namespace opentxs
{
BailmentNotice::BailmentNotice(
    const ConstNym& nym,
    const proto::PeerRequest& serialized)
        : ot_super(nym, serialized)
        , unit_(serialized.pendingbailment().unitid())
        , server_(serialized.pendingbailment().serverid())
        , txid_(serialized.pendingbailment().txid())
{
}

BailmentNotice::BailmentNotice(
    const ConstNym& nym,
    const Identifier& recipientID,
    const Identifier& unitID,
    const Identifier& serverID,
    const std::string& txid)
        : ot_super(nym, recipientID, proto::PEERREQUEST_PENDINGBAILMENT)
        , unit_(unitID)
        , server_(serverID)
        , txid_(txid)
{
}

proto::PeerRequest BailmentNotice::IDVersion() const
{
    auto contract = ot_super::IDVersion();

    auto& pendingbailment = *contract.mutable_pendingbailment();
    pendingbailment.set_version(version_);
    pendingbailment.set_unitid(String(unit_).Get());
    pendingbailment.set_serverid(String(server_).Get());
    pendingbailment.set_txid(txid_);

    return contract;
}
} // namespace opentxs
