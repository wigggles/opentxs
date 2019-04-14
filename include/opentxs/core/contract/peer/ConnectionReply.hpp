// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_CONNECTIONREPLY_HPP
#define OPENTXS_CORE_CONTRACT_PEER_CONNECTIONREPLY_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/peer/PeerReply.hpp"

#include <string>

namespace opentxs
{

class ConnectionReply : public PeerReply
{
private:
    typedef PeerReply ot_super;
    friend class PeerReply;

    bool success_{false};
    std::string url_;
    std::string login_;
    std::string password_;
    std::string key_;

    proto::PeerReply IDVersion(const Lock& lock) const override;

    ConnectionReply(
        const api::Wallet& wallet,
        const Nym_p& nym,
        const proto::PeerReply& serialized);
    ConnectionReply(
        const api::Wallet& wallet,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key);
    ConnectionReply() = delete;

public:
    ~ConnectionReply() = default;
};
}  // namespace opentxs

#endif
