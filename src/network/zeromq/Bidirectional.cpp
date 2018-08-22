// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Bidirectional.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>
#include "Message.hpp"

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
        receiver_thread_.reset(new std::thread(&Bidirectional::thread, this));

        OT_ASSERT(receiver_thread_)
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

    bool receiving{true};

    while (receiving) {
        auto& frame = msg->AddFrame();
        const bool received = (-1 != zmq_msg_recv(frame, pull_socket_, 0));

        if (false == received) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Receive error: " << zmq_strerror(zmq_errno())
                  << std::endl;

            return false;
        }

        int option{0};
        std::size_t optionBytes{sizeof(option)};

        const bool haveOption =
            (-1 !=
             zmq_getsockopt(pull_socket_, ZMQ_RCVMORE, &option, &optionBytes));

        if (false == haveOption) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to check socket options error:\n"
                  << zmq_strerror(zmq_errno()) << std::endl;

            return false;
        }

        OT_ASSERT(optionBytes == sizeof(option))

        if (1 != option) { receiving = false; }
    }

    auto sent = send(msg);

    return sent;
}

bool Bidirectional::process_receiver_socket()
{
    Lock lock(receiver_lock_, std::try_to_lock);
    if (!lock.owns_lock()) { return false; }

    auto reply = Message::Factory();

    bool receiving{true};

    while (receiving) {
        auto& frame = reply->AddFrame();
        const bool received = (-1 != zmq_msg_recv(frame, receiver_socket_, 0));

        if (false == received) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Receive error: " << zmq_strerror(zmq_errno())
                  << std::endl;

            return false;
        }

        int option{0};
        std::size_t optionBytes{sizeof(option)};

        const bool haveOption =
            (-1 != zmq_getsockopt(
                       receiver_socket_, ZMQ_RCVMORE, &option, &optionBytes));

        if (false == haveOption) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to check socket options error:\n"
                  << zmq_strerror(zmq_errno()) << std::endl;

            return false;
        }

        OT_ASSERT(optionBytes == sizeof(option))

        if (1 != option) { receiving = false; }
    }

    process_incoming(lock, reply);

    lock.unlock();

    return true;
}

bool Bidirectional::queue_message(zeromq::Message& message) const
{
    OT_ASSERT(nullptr != push_socket_);

    bool sent{true};
    const auto parts = message.size();
    std::size_t counter{0};

    for (auto& frame : message) {
        int flags{0};

        if (++counter < parts) { flags = ZMQ_SNDMORE; }

        sent |= (-1 != zmq_msg_send(frame, push_socket_, flags));
    }

    if (false == sent) {
        otErr << OT_METHOD << __FUNCTION__ << ": Send error:\n"
              << zmq_strerror(zmq_errno()) << std::endl;
    }

    return (false != sent);
}

bool Bidirectional::send(zeromq::Message& message)
{
    bool sent{true};
    const auto parts = message.size();
    std::size_t counter{0};

    for (auto& frame : message) {
        int flags{0};

        if (++counter < parts) { flags = ZMQ_SNDMORE; }

        sent |= (-1 != zmq_msg_send(frame, receiver_socket_, flags));
    }

    if (false == sent) {
        otErr << OT_METHOD << __FUNCTION__ << ": Send error:\n"
              << zmq_strerror(zmq_errno()) << std::endl;
    }

    return (false != sent);
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
