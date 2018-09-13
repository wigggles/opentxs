// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

#include "Message.hpp"
#include "Socket.hpp"

#include "Bidirectional.hpp"

#define CALLBACK_WAIT_MILLISECONDS 50
#define POLL_MILLISECONDS 1000
#define INPROC_PREFIX "inproc://opentxs/"

#define OT_METHOD "opentxs::network::zeromq::implementation::Bidirectional::"

namespace opentxs::network::zeromq::implementation
{
Bidirectional::Bidirectional(
    const zeromq::Context& context,
    std::mutex& lock,
    void* socket,
    const bool startThread)
    : Receiver(lock, socket, false)
    , push_socket_{zmq_socket(context, ZMQ_PUSH)}
    , endpoint_{INPROC_PREFIX}
    , pull_socket_{zmq_socket(context, ZMQ_PULL)}
{
    endpoint_.append(Identifier::Random()->str());

    OT_ASSERT(nullptr != pull_socket_);
    OT_ASSERT(nullptr != push_socket_);

    auto bound = bind(pull_socket_, receiver_lock_, endpoint_);

    OT_ASSERT(false != bound);

    auto connected = connect(push_socket_, receiver_lock_, endpoint_);

    OT_ASSERT(false != connected);

    if (startThread) {
        receiver_thread_ = std::thread(&Bidirectional::thread, this);
    }
}

bool Bidirectional::apply_timeouts(void* socket, std::mutex& socket_mutex) const
{
    OT_ASSERT(nullptr != socket);
    Lock lock(socket_mutex);

    auto set = zmq_setsockopt(socket, ZMQ_LINGER, &linger_, sizeof(linger_));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_LINGER"
              << std::endl;

        return false;
    }

    set = zmq_setsockopt(
        socket, ZMQ_SNDTIMEO, &send_timeout_, sizeof(send_timeout_));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_SNDTIMEO"
              << std::endl;

        return false;
    }

    set = zmq_setsockopt(
        socket, ZMQ_RCVTIMEO, &receive_timeout_, sizeof(receive_timeout_));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_RCVTIMEO"
              << std::endl;

        return false;
    }

    return true;
}

bool Bidirectional::bind(
    void* socket,
    std::mutex& socket_mutex,
    std::string& endpoint) const
{
    apply_timeouts(socket, socket_mutex);

    return (0 == zmq_bind(socket, endpoint.c_str()));
}

bool Bidirectional::connect(
    void* socket,
    std::mutex& socket_mutex,
    std::string& endpoint) const
{
    apply_timeouts(socket, socket_mutex);

    return (0 == zmq_connect(socket, endpoint.c_str()));
}

bool Bidirectional::process_pull_socket()
{
    Lock lock(receiver_lock_, std::try_to_lock);

    if (!lock.owns_lock()) { return false; }

    auto msg = Message::Factory();
    const auto received = Socket::receive_message(lock, pull_socket_, msg);

    if (false == received) { return false; }

    const auto sent = send(lock, msg);

    return sent;
}

bool Bidirectional::process_receiver_socket()
{
    Lock lock(receiver_lock_, std::try_to_lock);

    if (!lock.owns_lock()) { return false; }

    auto reply = Message::Factory();
    const auto received =
        Socket::receive_message(lock, receiver_socket_, reply);

    if (false == received) { return false; }

    process_incoming(lock, reply);
    lock.unlock();

    return true;
}

bool Bidirectional::queue_message(zeromq::Message& message) const
{
    OT_ASSERT(nullptr != push_socket_);

    Lock lock(send_lock_);

    return Socket::send_message(lock, push_socket_, message);
}

bool Bidirectional::send(const Lock& lock, zeromq::Message& message)
{
    return Socket::send_message(lock, receiver_socket_, message);
}

void Bidirectional::thread()
{
    otInfo << OT_METHOD << __FUNCTION__ << ": Starting listener" << std::endl;

    while (receiver_run_.get()) {
        if (have_callback()) { break; }

        Log::Sleep(std::chrono::milliseconds(CALLBACK_WAIT_MILLISECONDS));
    }

    otInfo << OT_METHOD << __FUNCTION__ << ": Callback ready" << std::endl;
    zmq_pollitem_t poll[2];

    while (receiver_run_.get()) {
        poll[0].socket = receiver_socket_;
        poll[0].events = ZMQ_POLLIN;
        poll[1].socket = pull_socket_;
        poll[1].events = ZMQ_POLLIN;
        const auto events = zmq_poll(poll, 2, POLL_MILLISECONDS);

        if (0 == events) {
            otInfo << OT_METHOD << __FUNCTION__ << ": No messages."
                   << std::endl;

            continue;
        }

        if (-1 == events) {
            const auto error = zmq_errno();
            otErr << OT_METHOD << __FUNCTION__
                  << ": Poll error: " << zmq_strerror(error) << std::endl;

            continue;
        }

        bool processed = true;
        if (ZMQ_POLLIN == poll[0].revents) {
            processed = process_receiver_socket();
        }

        if (false != processed && ZMQ_POLLIN == poll[1].revents) {
            processed = process_pull_socket();
        }

        if (false == processed) { return; }
    }

    otInfo << OT_METHOD << __FUNCTION__ << ": Shutting down" << std::endl;
}

Bidirectional::~Bidirectional() {}
}  // namespace opentxs::network::zeromq::implementation
