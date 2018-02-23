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
#define MESSAGE_CHECK_MICROSECONDS 1000

#define OT_METHOD "opentxs::network::zeromq::implementation::Receiver::"

namespace opentxs::network::zeromq::implementation
{
Receiver::Receiver(std::mutex& lock, void* socket)
    : receiver_lock_(lock)
    , receiver_socket_(socket)
    , receiver_run_(Flag::Factory(true))
    , receiver_thread_(nullptr)
{
    receiver_thread_.reset(new std::thread(&Receiver::thread, this));

    OT_ASSERT(receiver_thread_)
}

void Receiver::thread()
{
    while (receiver_run_.get()) {
        if (have_callback()) {
            break;
        }

        Log::Sleep(std::chrono::milliseconds(CALLBACK_WAIT_MILLISECONDS));
    }

    while (receiver_run_.get()) {
        Lock lock(receiver_lock_);
        auto request = Message::Factory();
        Message& message = request;
        auto status =
            (-1 != zmq_msg_recv(message, receiver_socket_, ZMQ_DONTWAIT));

        if (status) {
            process_incoming(lock, request);
        } else {
            const auto error = zmq_errno();

            if (EAGAIN != error) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Receive error: " << zmq_strerror(error)
                      << std::endl;
            }
        }

        lock.unlock();
        Log::Sleep(std::chrono::microseconds(MESSAGE_CHECK_MICROSECONDS));
    }
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
