// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_OUTBAILMENTREQUEST_HPP
#define OPENTXS_CORE_CONTRACT_PEER_OUTBAILMENTREQUEST_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

class OutBailmentRequest final : public PeerRequest
{
private:
    using ot_super = PeerRequest;
    friend class PeerRequest;

    OTUnitID unit_;
    OTServerID server_;
    Amount amount_{0};

    proto::PeerRequest IDVersion(const Lock& lock) const final;

    OutBailmentRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized);
    OutBailmentRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const std::uint64_t& amount,
        const std::string& terms);
    OutBailmentRequest() = delete;

public:
    ~OutBailmentRequest() final = default;
};
}  // namespace opentxs
#endif
