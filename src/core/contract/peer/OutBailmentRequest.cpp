// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/String.hpp"

#include "internal/api/Api.hpp"

#define CURRENT_VERSION 4

namespace opentxs
{
OutBailmentRequest::OutBailmentRequest(
    const api::internal::Core& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized)
    : ot_super(api, nym, serialized, serialized.outbailment().instructions())
    , unit_(api_.Factory().UnitID(serialized.outbailment().unitid()))
    , server_(api_.Factory().ServerID(serialized.outbailment().serverid()))
    , amount_(serialized.outbailment().amount())
{
}

OutBailmentRequest::OutBailmentRequest(
    const api::internal::Core& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Server& serverID,
    const std::uint64_t& amount,
    const std::string& terms)
    : ot_super(
          api,
          nym,
          CURRENT_VERSION,
          recipientID,
          serverID,
          terms,
          proto::PEERREQUEST_OUTBAILMENT)
    , unit_(unitID)
    , server_(serverID)
    , amount_(amount)
{
}

proto::PeerRequest OutBailmentRequest::IDVersion(const Lock& lock) const
{
    auto contract = ot_super::IDVersion(lock);
    auto& outbailment = *contract.mutable_outbailment();
    outbailment.set_version(version_);
    outbailment.set_unitid(String::Factory(unit_)->Get());
    outbailment.set_serverid(String::Factory(server_)->Get());
    outbailment.set_amount(amount_);
    outbailment.set_instructions(conditions_);

    return contract;
}
}  // namespace opentxs
