// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"
#include "stdafx.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/Types.hpp"

#include <zmq.h>

#include "Socket.hpp"

#include <memory>
#include <mutex>
#include <thread>

#define CALLBACK_WAIT_MILLISECONDS 50
#define RECEIVER_POLL_MILLISECONDS 1000

#define RECEIVER_METHOD "opentxs::network::zeromq::implementation::Receiver::"

namespace opentxs::network::zeromq::implementation
{
template <typename T>
class Receiver
{
protected:
    std::mutex& receiver_lock_;
    // Not owned by this class
    void* receiver_socket_{nullptr};
    OTFlag receiver_run_;
    std::unique_ptr<std::thread> receiver_thread_{nullptr};

    virtual bool have_callback() const { return false; }

    virtual void process_incoming(const Lock& lock, T& message) = 0;
    virtual void thread()
    {
        otInfo << RECEIVER_METHOD << __FUNCTION__ << ": Starting listener"
               << std::endl;

        while (receiver_run_.get()) {
            if (have_callback()) { break; }

            Log::Sleep(std::chrono::milliseconds(CALLBACK_WAIT_MILLISECONDS));
        }

        otInfo << RECEIVER_METHOD << __FUNCTION__ << ": Callback ready"
               << std::endl;
        zmq_pollitem_t poll[1];

        while (receiver_run_.get()) {
            poll[0].socket = receiver_socket_;
            poll[0].events = ZMQ_POLLIN;
            const auto events = zmq_poll(poll, 1, RECEIVER_POLL_MILLISECONDS);

            if (0 == events) {
                otInfo << RECEIVER_METHOD << __FUNCTION__ << ": No messages."
                       << std::endl;

                continue;
            }

            if (-1 == events) {
                const auto error = zmq_errno();
                otErr << RECEIVER_METHOD << __FUNCTION__
                      << ": Poll error: " << zmq_strerror(error) << std::endl;

                continue;
            }

            Lock lock(receiver_lock_, std::try_to_lock);

            if (!lock.owns_lock()) { return; }

            auto reply = T::Factory();
            const auto received =
                Socket::receive_message(lock, receiver_socket_, reply);

            if (false == received) { return; }

            process_incoming(lock, reply);
            lock.unlock();
        }

        otInfo << RECEIVER_METHOD << __FUNCTION__ << ": Shutting down"
               << std::endl;
    }

    Receiver(std::mutex& lock, void* socket, const bool startThread)
        : receiver_lock_(lock)
        , receiver_socket_(socket)
        , receiver_run_(Flag::Factory(true))
        , receiver_thread_(nullptr)
    {
        if (startThread) {
            receiver_thread_.reset(new std::thread(&Receiver::thread, this));

            OT_ASSERT(receiver_thread_)
        }
    }

    virtual ~Receiver()
    {
        Lock lock(receiver_lock_);
        receiver_run_->Off();

        if (receiver_thread_ && receiver_thread_->joinable()) {
            receiver_thread_->join();
            receiver_thread_.reset();
        }

        receiver_socket_ = nullptr;
    }

private:
    Receiver() = delete;
    Receiver(const Receiver&) = delete;
    Receiver(Receiver&&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver& operator=(Receiver&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
