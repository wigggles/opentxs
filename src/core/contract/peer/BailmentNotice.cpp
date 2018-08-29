// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/peer/BailmentNotice.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"

#define CURRENT_VERSION 6

namespace opentxs
{
BailmentNotice::BailmentNotice(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::PeerRequest& serialized)
    : ot_super(wallet, nym, serialized)
    , unit_(Identifier::Factory(serialized.pendingbailment().unitid()))
    , server_(Identifier::Factory(serialized.pendingbailment().serverid()))
    , requestID_(Identifier::Factory(serialized.pendingbailment().requestid()))
    , txid_(serialized.pendingbailment().txid())
    , amount_(serialized.pendingbailment().amount())
{
}

BailmentNotice::BailmentNotice(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const Identifier& recipientID,
    const Identifier& unitID,
    const Identifier& serverID,
    const Identifier& requestID,
    const std::string& txid,
    const Amount& amount)
    : ot_super(
          wallet,
          nym,
          CURRENT_VERSION,
          recipientID,
          serverID,
          proto::PEERREQUEST_PENDINGBAILMENT)
    , unit_(Identifier::Factory(unitID))
    , server_(Identifier::Factory(serverID))
    , requestID_(Identifier::Factory(requestID))
    , txid_(txid)
    , amount_(amount)
{
}

proto::PeerRequest BailmentNotice::IDVersion(const Lock& lock) const
{
    auto contract = ot_super::IDVersion(lock);

    auto& pendingbailment = *contract.mutable_pendingbailment();
    pendingbailment.set_version(version_);
    pendingbailment.set_unitid(String::Factory(unit_)->Get());
    pendingbailment.set_serverid(String::Factory(server_)->Get());
    pendingbailment.set_requestid(String::Factory(requestID_)->Get());
    pendingbailment.set_txid(txid_);
    pendingbailment.set_amount(amount_);

    return contract;
}
}  // namespace opentxs
