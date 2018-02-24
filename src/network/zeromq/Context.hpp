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

#include "opentxs/Internal.hpp"

#include "opentxs/network/zeromq/Context.hpp"

namespace opentxs::network::zeromq::implementation
{
class Context : virtual public zeromq::Context
{
public:
    operator void*() const override;

    OTZMQPublishSocket PublishSocket() const override;
    OTZMQReplySocket ReplySocket(const ReplyCallback& callback) const override;
    OTZMQRequestSocket RequestSocket() const override;
    OTZMQSubscribeSocket SubscribeSocket(
        const ListenCallback& callback) const override;

    ~Context();

private:
    friend network::zeromq::Context;

    void* context_{nullptr};

    Context* clone() const override;

    Context();
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_CONTEXT_HPP
