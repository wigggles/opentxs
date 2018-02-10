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

#ifndef OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_MESSAGE_HPP
#define OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_MESSAGE_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/network/zeromq/Message.hpp"

#include <string>

struct zmq_msg_t;

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace implementation
{

class Message : virtual public zeromq::Message
{
public:
    operator std::string() const override;

    const void* data() const override;
    std::size_t size() const override;

    operator zmq_msg_t*() override;

    ~Message();

private:
    friend class Context;

    zmq_msg_t* message_{nullptr};

    Message();
    explicit Message(const Data& input);
    explicit Message(const std::string& input);
    Message(const Message&) = delete;
    Message(Message&&) = delete;
    Message& operator=(Message&&) = delete;
    Message& operator=(const Message&) = delete;
};
}  // namespace implementation
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_MESSAGE_HPP
