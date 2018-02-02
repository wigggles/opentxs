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

#include "opentxs/core/contract/peer/OutBailmentReply.hpp"

#define CURRENT_VERSION 4

namespace opentxs
{
OutBailmentReply::OutBailmentReply(
    const ConstNym& nym,
    const proto::PeerReply& serialized)
    : ot_super(nym, serialized)
{
    conditions_ = serialized.outbailment().instructions();
}

OutBailmentReply::OutBailmentReply(
    const ConstNym& nym,
    const Identifier& initiator,
    const Identifier& request,
    const Identifier& server,
    const std::string& terms)
    : ot_super(
          nym,
          CURRENT_VERSION,
          initiator,
          server,
          proto::PEERREQUEST_OUTBAILMENT,
          request)
{
    conditions_ = terms;
}

proto::PeerReply OutBailmentReply::IDVersion(const Lock& lock) const
{
    auto contract = ot_super::IDVersion(lock);

    auto& bailment = *contract.mutable_outbailment();
    bailment.set_version(version_);
    bailment.set_instructions(conditions_);

    return contract;
}
}  // namespace opentxs
