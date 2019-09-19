// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_BAILMENTNOTICE_HPP
#define OPENTXS_CORE_CONTRACT_PEER_BAILMENTNOTICE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

#include <string>

namespace opentxs
{
class BailmentNotice final : public PeerRequest
{
public:
    ~BailmentNotice() final = default;

private:
    typedef PeerRequest ot_super;
    friend class PeerRequest;

    OTUnitID unit_;
    OTServerID server_;
    OTIdentifier requestID_;
    std::string txid_;
    Amount amount_;

    proto::PeerRequest IDVersion(const Lock& lock) const final;

    BailmentNotice(
        const api::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized);
    BailmentNotice(
        const api::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const Identifier& requestID,
        const std::string& txid,
        const Amount& amount);
    BailmentNotice() = delete;
};
}  // namespace opentxs
#endif
