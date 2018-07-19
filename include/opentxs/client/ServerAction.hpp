// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_SERVER_ACTION_HPP
#define OPENTXS_CLIENT_SERVER_ACTION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Types.hpp"

#include <memory>
#include <string>

namespace opentxs
{
class Message;
class PeerReply;
class PeerRequest;

namespace client
{
class ServerAction
{
public:
    EXPORT virtual TransactionNumber GetTransactionNumber() const = 0;
    EXPORT virtual SendResult LastSendResult() const = 0;
    EXPORT virtual const OTIdentifier MessageID() const = 0;
    EXPORT virtual const std::shared_ptr<PeerRequest>& SentPeerRequest()
        const = 0;
    EXPORT virtual const std::shared_ptr<PeerReply>& SentPeerReply() const = 0;
    EXPORT virtual const std::shared_ptr<Message>& Reply() const = 0;

    EXPORT virtual std::string Run(const std::size_t totalRetries = 2) = 0;

    EXPORT virtual ~ServerAction() = default;

protected:
    ServerAction() = default;

private:
    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction&&) = delete;
    ServerAction& operator=(const ServerAction&) = delete;
    ServerAction& operator=(ServerAction&&) = delete;
};
}  // namespace client
}  // namespace opentxs
#endif  // OPENTXS_CLIENT_SERVER_ACTION_HPP
