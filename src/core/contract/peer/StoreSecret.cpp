// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/peer/StoreSecret.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"

#define CURRENT_VERSION 4

namespace opentxs
{
StoreSecret::StoreSecret(
    const api::internal::Core& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized)
    : ot_super(api, nym, serialized)
    , secret_type_(serialized.storesecret().type())
    , primary_(serialized.storesecret().primary())
    , secondary_(serialized.storesecret().secondary())
{
}

StoreSecret::StoreSecret(
    const api::internal::Core& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const proto::SecretType type,
    const std::string& primary,
    const std::string& secondary,
    const identifier::Server& serverID)
    : ot_super(
          api,
          nym,
          CURRENT_VERSION,
          recipientID,
          serverID,
          proto::PEERREQUEST_STORESECRET)
    , secret_type_(type)
    , primary_(primary)
    , secondary_(secondary)
{
}

proto::PeerRequest StoreSecret::IDVersion(const Lock& lock) const
{
    auto contract = ot_super::IDVersion(lock);

    auto& storesecret = *contract.mutable_storesecret();
    storesecret.set_version(version_);
    storesecret.set_type(secret_type_);
    storesecret.set_primary(primary_);
    storesecret.set_secondary(secondary_);

    return contract;
}
}  // namespace opentxs
