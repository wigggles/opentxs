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

#ifndef OPENTXS_NETWORK_ZEROMQ_MESSAGE_HPP
#define OPENTXS_NETWORK_ZEROMQ_MESSAGE_HPP

#include "opentxs/Forward.hpp"

#include <string>

struct zmq_msg_t;

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::Message::operator zmq_msg_t*();
%template(OTZMQMessage) opentxs::Pimpl<opentxs::network::zeromq::Message>;
%rename(string) opentxs::network::zeromq::Message::operator std::string() const;
%rename(ZMQMessage) opentxs::network::zeromq::Message;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message
{
public:
    EXPORT static Pimpl<opentxs::network::zeromq::Message> Factory();
    EXPORT static Pimpl<opentxs::network::zeromq::Message> Factory(
        const opentxs::Data& input);
    EXPORT static Pimpl<opentxs::network::zeromq::Message> Factory(
        const std::string& input);

    EXPORT virtual operator std::string() const = 0;

    EXPORT virtual const void* data() const = 0;
    EXPORT virtual std::size_t size() const = 0;

    EXPORT virtual operator zmq_msg_t*() = 0;

    EXPORT virtual ~Message() = default;

protected:
    Message() = default;

private:
    friend OTZMQMessage;

    EXPORT virtual Message* clone() const = 0;

    Message(const Message&) = delete;
    Message(Message&&) = delete;
    Message& operator=(Message&&) = delete;
    Message& operator=(const Message&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_MESSAGE_HPP
