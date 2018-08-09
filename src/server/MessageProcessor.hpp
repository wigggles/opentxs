// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/network/zeromq/Socket.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace opentxs
{
namespace server
{
class MessageProcessor : Lockable
{
public:
    EXPORT explicit MessageProcessor(
        Server& server,
        const network::zeromq::Context& context,
        const Flag& running);

    EXPORT void cleanup();
    EXPORT void init(
        const bool inproc,
        const int port,
        const OTPassword& privkey);
    EXPORT void Start();

    EXPORT ~MessageProcessor();

private:
    Server& server_;
    const Flag& running_;
    [[maybe_unused]] const network::zeromq::Context& context_;
    OTZMQReplyCallback reply_socket_callback_;
    OTZMQReplySocket reply_socket_;
    std::unique_ptr<std::thread> thread_{nullptr};

    bool processMessage(const std::string& messageString, std::string& reply);
    OTZMQMessage processSocket(const network::zeromq::Message& incoming);
    void run();

    MessageProcessor() = delete;
};
}  // namespace server
}  // namespace opentxs
