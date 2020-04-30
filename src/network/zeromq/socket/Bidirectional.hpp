// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include <string>

#include "network/zeromq/socket/Receiver.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Sender.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
template <typename InterfaceType, typename MessageType = zeromq::Message>
class Bidirectional
    : virtual public Receiver<InterfaceType, MessageType>,
      virtual public Sender<InterfaceType, Receiver<InterfaceType, MessageType>>
{
protected:
    Bidirectional(
        const zeromq::Context& context,
        const bool startThread) noexcept;

    void init() noexcept override;
    void shutdown(const Lock& lock) noexcept final;

    ~Bidirectional() override = default;

private:
    const bool bidirectional_start_thread_;
    const std::string endpoint_;
    void* push_socket_;
    void* pull_socket_;
    mutable int linger_;
    mutable int send_timeout_;
    mutable int receive_timeout_;
    mutable std::mutex send_lock_;

    bool apply_timeouts(void* socket, std::mutex& socket_mutex) const noexcept;
    bool bind(
        void* socket,
        std::mutex& socket_mutex,
        const std::string& endpoint) const noexcept;
    bool connect(
        void* socket,
        std::mutex& socket_mutex,
        const std::string& endpoint) const noexcept;

    bool process_pull_socket(const Lock& lock) noexcept;
    bool process_receiver_socket(const Lock& lock) noexcept;
    bool send(zeromq::Message& message) const noexcept final;
    bool send(const Lock& lock, zeromq::Message& message) noexcept;
    void thread() noexcept final;

    Bidirectional() = delete;
    Bidirectional(const Bidirectional&) = delete;
    Bidirectional(Bidirectional&&) = delete;
    Bidirectional& operator=(const Bidirectional&) = delete;
    Bidirectional& operator=(Bidirectional&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
