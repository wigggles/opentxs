// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/peer/ConnectionRequest.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"

#define CURRENT_VERSION 4

namespace opentxs
{
ConnectionRequest::ConnectionRequest(
    const api::Core& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized)
    : ot_super(api, nym, serialized)
    , connection_type_(serialized.connectioninfo().type())
{
}

ConnectionRequest::ConnectionRequest(
    const api::Core& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const proto::ConnectionInfoType type,
    const identifier::Server& serverID)
    : ot_super(
          api,
          nym,
          CURRENT_VERSION,
          recipientID,
          serverID,
          proto::PEERREQUEST_CONNECTIONINFO)
    , connection_type_(type)
{
}

proto::PeerRequest ConnectionRequest::IDVersion(const Lock& lock) const
{
    auto contract = ot_super::IDVersion(lock);

    auto& connectioninfo = *contract.mutable_connectioninfo();
    connectioninfo.set_version(version_);
    connectioninfo.set_type(connection_type_);

    return contract;
}
}  // namespace opentxs
