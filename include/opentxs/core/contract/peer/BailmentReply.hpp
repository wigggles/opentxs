// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_BAILMENTREPLY_HPP
#define OPENTXS_CORE_CONTRACT_PEER_BAILMENTREPLY_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/peer/PeerReply.hpp"

#include <string>

namespace opentxs
{

class BailmentReply : public PeerReply
{
private:
    typedef PeerReply ot_super;
    friend class PeerReply;

    proto::PeerReply IDVersion(const Lock& lock) const override;

    BailmentReply(
        const api::Wallet& wallet,
        const ConstNym& nym,
        const proto::PeerReply& serialized);
    BailmentReply(
        const api::Wallet& wallet,
        const ConstNym& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const std::string& terms);
    BailmentReply() = delete;

public:
    ~BailmentReply() = default;
};
}  // namespace opentxs

#endif
