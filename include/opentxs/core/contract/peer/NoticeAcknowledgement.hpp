// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_NOTICEACKNOWLEDGEMENT_HPP
#define OPENTXS_CORE_CONTRACT_PEER_NOTICEACKNOWLEDGEMENT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/peer/PeerReply.hpp"

#include <string>

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

class NoticeAcknowledgement final : public PeerReply
{
private:
    typedef PeerReply ot_super;
    friend class PeerReply;

    bool ack_{false};

    proto::PeerReply IDVersion(const Lock& lock) const final;

    NoticeAcknowledgement(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized);
    NoticeAcknowledgement(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const proto::PeerRequestType type,
        const bool& ack);
    NoticeAcknowledgement() = delete;

public:
    ~NoticeAcknowledgement() final = default;
};
}  // namespace opentxs

#endif
