// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/Types.hpp"

#include "Receiver.hpp"

#include <memory>
#include <mutex>
#include <thread>

namespace opentxs::network::zeromq::implementation
{
class Bidirectional : public Receiver<zeromq::Message>
{
protected:
    void* push_socket_{nullptr};

    Bidirectional(
        const zeromq::Context& context,
        std::mutex& lock,
        const Flag& running,
        void* socket,
        const bool startThread);

    bool queue_message(zeromq::Message& message) const;

    virtual ~Bidirectional();

private:
    const std::string endpoint_;
    void* pull_socket_{nullptr};
    mutable int linger_{0};
    mutable int send_timeout_{-1};
    mutable int receive_timeout_{-1};
    mutable std::mutex send_lock_;

    bool apply_timeouts(void* socket, std::mutex& socket_mutex) const;
    bool bind(
        void* socket,
        std::mutex& socket_mutex,
        const std::string& endpoint) const;
    bool connect(
        void* socket,
        std::mutex& socket_mutex,
        const std::string& endpoint) const;
    bool process_pull_socket();
    bool process_receiver_socket();
    bool send(const Lock& lock, zeromq::Message& message);
    void thread() override;

    Bidirectional() = delete;
    Bidirectional(const Bidirectional&) = delete;
    Bidirectional(Bidirectional&&) = delete;
    Bidirectional& operator=(const Bidirectional&) = delete;
    Bidirectional& operator=(Bidirectional&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
