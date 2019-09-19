// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/peer/ConnectionReply.hpp"

#include "opentxs/core/Identifier.hpp"

#define CURRENT_VERSION 4

namespace opentxs
{
ConnectionReply::ConnectionReply(
    const api::Core& api,
    const Nym_p& nym,
    const proto::PeerReply& serialized)
    : ot_super(api, nym, serialized)
    , success_(serialized.connectioninfo().success())
    , url_(serialized.connectioninfo().url())
    , login_(serialized.connectioninfo().login())
    , password_(serialized.connectioninfo().password())
    , key_(serialized.connectioninfo().key())
{
}

ConnectionReply::ConnectionReply(
    const api::Core& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const Identifier& request,
    const identifier::Server& server,
    const bool ack,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& key)
    : ot_super(
          api,
          nym,
          CURRENT_VERSION,
          initiator,
          server,
          proto::PEERREQUEST_CONNECTIONINFO,
          request)
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
}  // namespace opentxs
