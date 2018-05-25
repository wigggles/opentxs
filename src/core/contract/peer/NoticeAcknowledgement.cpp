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

#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"

#include "opentxs/core/Identifier.hpp"

#define CURRENT_VERSION 4

namespace opentxs
{
NoticeAcknowledgement::NoticeAcknowledgement(
    const ConstNym& nym,
    const proto::PeerReply& serialized)
    : ot_super(nym, serialized)
    , ack_(serialized.notice().ack())
{
}

NoticeAcknowledgement::NoticeAcknowledgement(
    const ConstNym& nym,
    const Identifier& initiator,
    const Identifier& request,
    const Identifier& server,
    const proto::PeerRequestType type,
    const bool& ack)
    : ot_super(nym, CURRENT_VERSION, initiator, server, type, request)
    , ack_(ack)
{
}

proto::PeerReply NoticeAcknowledgement::IDVersion(const Lock& lock) const
{
    auto contract = ot_super::IDVersion(lock);

    auto& notice = *contract.mutable_notice();
    notice.set_version(version_);
    notice.set_ack(ack_);

    return contract;
}
}  // namespace opentxs
