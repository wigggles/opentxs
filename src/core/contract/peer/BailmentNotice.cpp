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

#include "opentxs/core/contract/peer/BailmentNotice.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"

#define CURRENT_VERSION 6

namespace opentxs
{
BailmentNotice::BailmentNotice(
    const ConstNym& nym,
    const proto::PeerRequest& serialized)
    : ot_super(nym, serialized)
    , unit_(Identifier::Factory(serialized.pendingbailment().unitid()))
    , server_(Identifier::Factory(serialized.pendingbailment().serverid()))
    , requestID_(Identifier::Factory(serialized.pendingbailment().requestid()))
    , txid_(serialized.pendingbailment().txid())
    , amount_(serialized.pendingbailment().amount())
{
}

BailmentNotice::BailmentNotice(
    const ConstNym& nym,
    const Identifier& recipientID,
    const Identifier& unitID,
    const Identifier& serverID,
    const Identifier& requestID,
    const std::string& txid,
    const Amount& amount)
    : ot_super(
          nym,
          CURRENT_VERSION,
          recipientID,
          serverID,
          proto::PEERREQUEST_PENDINGBAILMENT)
    , unit_(Identifier::Factory(unitID))
    , server_(Identifier::Factory(serverID))
    , requestID_(Identifier::Factory(requestID))
    , txid_(txid)
    , amount_(amount)
{
}

proto::PeerRequest BailmentNotice::IDVersion(const Lock& lock) const
{
    auto contract = ot_super::IDVersion(lock);

    auto& pendingbailment = *contract.mutable_pendingbailment();
    pendingbailment.set_version(version_);
    pendingbailment.set_unitid(String(unit_).Get());
    pendingbailment.set_serverid(String(server_).Get());
    pendingbailment.set_requestid(String(requestID_).Get());
    pendingbailment.set_txid(txid_);
    pendingbailment.set_amount(amount_);

    return contract;
}
}  // namespace opentxs
