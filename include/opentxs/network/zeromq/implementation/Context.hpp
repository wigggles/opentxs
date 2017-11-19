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

#ifndef OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_CONTEXT_HPP
#define OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_CONTEXT_HPP

#include "opentxs/Version.hpp"

#include "opentxs/network/zeromq/Context.hpp"

namespace opentxs
{
namespace api
{
namespace implementation
{
class Native;
}  // namespace implementation

namespace network
{
namespace implementation
{
class ZMQ;
}  // namespace implementation
}  // namespace network
}  // namespace api

namespace network
{
namespace zeromq
{
class ReplySocket;
class RequestSocket;

namespace implementation
{

class Context : virtual public zeromq::Context
{
public:
    operator void*() const override;

    std::shared_ptr<zeromq::Message> NewMessage() const override;
    std::shared_ptr<zeromq::Message> NewMessage(
        const Data& input) const override;
    std::shared_ptr<zeromq::Message> NewMessage(
        const std::string& input) const override;
    std::shared_ptr<zeromq::ReplySocket> NewReplySocket() const override;
    std::shared_ptr<zeromq::RequestSocket> NewRequestSocket() const override;

    ~Context();

private:
    friend class api::implementation::Native;
    friend class api::network::implementation::ZMQ;

    void* context_{nullptr};

    Context();
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
};
}  // namespace implementation
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_CONTEXT_HPP
