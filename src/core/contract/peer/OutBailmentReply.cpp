// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/peer/OutBailmentReply.hpp"

#include "opentxs/core/Identifier.hpp"

#define CURRENT_VERSION 4

namespace opentxs
{
OutBailmentReply::OutBailmentReply(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::PeerReply& serialized)
    : ot_super(wallet, nym, serialized)
{
    conditions_ = serialized.outbailment().instructions();
}

OutBailmentReply::OutBailmentReply(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const identifier::Nym& initiator,
    const Identifier& request,
    const identifier::Server& server,
    const std::string& terms)
    : ot_super(
          wallet,
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
