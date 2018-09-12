// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/peer/BailmentRequest.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"

#define CURRENT_VERSION 4

namespace opentxs
{
BailmentRequest::BailmentRequest(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::PeerRequest& serialized)
    : ot_super(wallet, nym, serialized)
    , unit_(Identifier::Factory(serialized.bailment().unitid()))
    , server_(Identifier::Factory(serialized.bailment().serverid()))
{
}

BailmentRequest::BailmentRequest(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const Identifier& recipientID,
    const Identifier& unitID,
    const Identifier& serverID)
    : ot_super(
          wallet,
          nym,
          CURRENT_VERSION,
          recipientID,
          serverID,
          proto::PEERREQUEST_BAILMENT)
    , unit_(Identifier::Factory(unitID))
    , server_(Identifier::Factory(serverID))
{
}

proto::PeerRequest BailmentRequest::IDVersion(const Lock& lock) const
{
    auto contract = ot_super::IDVersion(lock);
    auto& bailment = *contract.mutable_bailment();
    bailment.set_version(version_);
    bailment.set_unitid(String::Factory(unit_)->Get());
    bailment.set_serverid(String::Factory(server_)->Get());

    return contract;
}
}  // namespace opentxs
