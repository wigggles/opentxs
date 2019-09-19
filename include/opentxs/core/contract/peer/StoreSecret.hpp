// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_STORESECRET_HPP
#define OPENTXS_CORE_CONTRACT_PEER_STORESECRET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/peer/PeerRequest.hpp"

namespace opentxs
{

class StoreSecret final : public PeerRequest
{
private:
    typedef PeerRequest ot_super;
    friend class PeerRequest;

    proto::SecretType secret_type_;
    std::string primary_;
    std::string secondary_;

    proto::PeerRequest IDVersion(const Lock& lock) const final;

    StoreSecret(
        const api::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized);
    StoreSecret(
        const api::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const proto::SecretType type,
        const std::string& primary,
        const std::string& secondary,
        const identifier::Server& serverID);
    StoreSecret() = delete;

public:
    ~StoreSecret() final = default;
};
}  // namespace opentxs

#endif
