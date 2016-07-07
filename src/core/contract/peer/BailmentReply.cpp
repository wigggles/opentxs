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

#include "opentxs/core/contract/peer/BailmentReply.hpp"

namespace opentxs
{
BailmentReply::BailmentReply(
    const ConstNym& nym,
    const proto::PeerReply& serialized)
      : ot_super(nym, serialized)
{
    conditions_ = serialized.bailment().instructions();
}

BailmentReply::BailmentReply(
    const ConstNym& nym,
    const Identifier& initiator,
    const Identifier& request,
    const std::string& terms)
      : ot_super(nym, initiator, proto::PEERREQUEST_BAILMENT, request)
{
    conditions_ = terms;
}

proto::PeerReply BailmentReply::IDVersion() const
{
    auto contract = ot_super::IDVersion();

    auto& bailment = *contract.mutable_bailment();
    bailment.set_version(version_);
    bailment.set_instructions(conditions_);

    return contract;
}
} // namespace opentxs
