/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/stdafx.hpp"

#include "Receiver.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

#define CALLBACK_WAIT_MILLISECONDS 50
#define POLL_MILLISECONDS 1000

#define OT_METHOD "opentxs::network::zeromq::implementation::Receiver::"

namespace opentxs::network::zeromq::implementation
{
Receiver::Receiver(std::mutex& lock, void* socket, const bool startThread)
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

void Receiver::thread()
{
    otInfo << OT_METHOD << __FUNCTION__ << ": Starting listener" << std::endl;

    while (receiver_run_.get()) {
        if (have_callback()) {
            break;
        }

        Log::Sleep(std::chrono::milliseconds(CALLBACK_WAIT_MILLISECONDS));
    }

    otInfo << OT_METHOD << __FUNCTION__ << ": Callback ready" << std::endl;
    zmq_pollitem_t poll[1];

    while (receiver_run_.get()) {
        poll[0].socket = receiver_socket_;
        poll[0].events = ZMQ_POLLIN;
        const auto events = zmq_poll(poll, 1, POLL_MILLISECONDS);

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

        Lock lock(receiver_lock_);
        auto request = Message::Factory();
        Message& message = request;
        auto status = (-1 != zmq_msg_recv(message, receiver_socket_, 0));

        if (status) {
            process_incoming(lock, request);
        } else {
            const auto error = zmq_errno();
            otErr << OT_METHOD << __FUNCTION__
                  << ": Receive error: " << zmq_strerror(error) << std::endl;
        }

        lock.unlock();
    }

    otInfo << OT_METHOD << __FUNCTION__ << ": Shutting down" << std::endl;
}

Receiver::~Receiver()
{
    Lock lock(receiver_lock_);
    receiver_run_->Off();

    if (receiver_thread_ && receiver_thread_->joinable()) {
        receiver_thread_->join();
        receiver_thread_.reset();
    }

    receiver_socket_ = nullptr;
}
}  // namespace opentxs::network::zeromq::implementation
