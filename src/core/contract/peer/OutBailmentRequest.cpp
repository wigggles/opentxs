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

#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"

#include "opentxs/core/String.hpp"

namespace opentxs
{
OutBailmentRequest::OutBailmentRequest(
    const ConstNym& nym,
    const proto::PeerRequest& serialized)
        : ot_super(nym, serialized, serialized.outbailment().instructions())
        , unit_(serialized.outbailment().unitid())
        , server_(serialized.outbailment().serverid())
        , amount_(serialized.outbailment().amount())
{
}

OutBailmentRequest::OutBailmentRequest(
    const ConstNym& nym,
    const Identifier& recipientID,
    const Identifier& unitID,
    const Identifier& serverID,
    const std::uint64_t& amount,
    const std::string& terms)
        : ot_super(nym, recipientID, terms, proto::PEERREQUEST_OUTBAILMENT)
        , unit_(unitID)
        , server_(serverID)
        , amount_(amount)
{
}

proto::PeerRequest OutBailmentRequest::IDVersion() const
{
    auto contract = ot_super::IDVersion();

    auto& outbailment = *contract.mutable_outbailment();
    outbailment.set_version(version_);
    outbailment.set_unitid(String(unit_).Get());
    outbailment.set_serverid(String(server_).Get());
    outbailment.set_amount(amount_);
    outbailment.set_instructions(conditions_);

    return contract;
}
} // namespace opentxs
