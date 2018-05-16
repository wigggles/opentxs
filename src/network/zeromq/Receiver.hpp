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

#ifndef OPENTXS_NETWORK_ZEROMQ_RECEIVER_IMPLEMENTATION_HPP
#define OPENTXS_NETWORK_ZEROMQ_RECEIVER_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/Types.hpp"

#include <memory>
#include <mutex>
#include <thread>

namespace opentxs::network::zeromq::implementation
{
class Receiver
{
protected:
    Receiver(std::mutex& lock, void* socket, const bool startThread);

    virtual ~Receiver();

private:
    std::mutex& receiver_lock_;
    // Not owned by this class
    void* receiver_socket_{nullptr};
    OTFlag receiver_run_;
    std::unique_ptr<std::thread> receiver_thread_{nullptr};

    virtual bool have_callback() const { return false; }

    virtual void process_incoming(
        const Lock& lock,
        MultipartMessage& message) = 0;
    void thread();

    Receiver() = delete;
    Receiver(const Receiver&) = delete;
    Receiver(Receiver&&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver& operator=(Receiver&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_RECEIVER_IMPLEMENTATION_HPP
