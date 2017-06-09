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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/contract/peer/ConnectionReply.hpp"

namespace opentxs
{
ConnectionReply::ConnectionReply(
    const ConstNym& nym,
    const proto::PeerReply& serialized)
      : ot_super(nym, serialized)
      , success_(serialized.connectioninfo().success())
      , url_(serialized.connectioninfo().url())
      , login_(serialized.connectioninfo().login())
      , password_(serialized.connectioninfo().password())
      , key_(serialized.connectioninfo().key())
{
}

ConnectionReply::ConnectionReply(
    const ConstNym& nym,
    const Identifier& initiator,
    const Identifier& request,
    const Identifier& server,
    const bool ack,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& key)
      : ot_super(
          nym, initiator, server, proto::PEERREQUEST_CONNECTIONINFO, request)
      , success_(ack)
      , url_(url)
      , login_(login)
      , password_(password)
      , key_(key)
{
}

proto::PeerReply ConnectionReply::IDVersion(const Lock& lock) const
{
    auto contract = ot_super::IDVersion(lock);

    auto& connectioninfo = *contract.mutable_connectioninfo();
    connectioninfo.set_version(version_);
    connectioninfo.set_success(success_);
    connectioninfo.set_url(url_);
    connectioninfo.set_login(login_);
    connectioninfo.set_password(password_);
    connectioninfo.set_key(key_);

    return contract;
}
} // namespace opentxs
