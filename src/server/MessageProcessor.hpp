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

namespace opentxs::server
{
class MessageProcessor : Lockable
{
public:
    explicit MessageProcessor(
        Server& server,
        const network::zeromq::Context& context,
        const Flag& running);

    void cleanup();
    void init(const bool inproc, const int port, const OTPassword& privkey);
    void Start();

    ~MessageProcessor();

private:
    Server& server_;
    const Flag& running_;
    [[maybe_unused]] const network::zeromq::Context& context_;
    OTZMQListenCallback frontend_callback_;
    OTZMQRouterSocket frontend_socket_;
    OTZMQReplyCallback backend_callback_;
    OTZMQReplySocket backend_socket_;
    OTZMQListenCallback internal_callback_;
    OTZMQDealerSocket internal_socket_;
    std::unique_ptr<std::thread> thread_{nullptr};
    const std::string internal_endpoint_;

    void process_frontend(const network::zeromq::Message& incoming);
    void process_internal(const network::zeromq::Message& incoming);
    bool processMessage(const std::string& messageString, std::string& reply);
    OTZMQMessage process_backend(const network::zeromq::Message& incoming);
    void run();

    MessageProcessor() = delete;
};
}  // namespace opentxs::server
