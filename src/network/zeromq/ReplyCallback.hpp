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

#ifndef OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_IMPLEMENTATION_HPP
#define OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_IMPLEMENTATION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/ReplyCallback.hpp"

namespace opentxs::network::zeromq::implementation
{
class ReplyCallback : virtual public zeromq::ReplyCallback
{
public:
    OTZMQMultipartMessage Process(
        const zeromq::MultipartMessage& message) const override;

    ~ReplyCallback();

private:
    friend zeromq::ReplyCallback;

    const zeromq::ReplyCallback::ReceiveCallback callback_;

    ReplyCallback* clone() const override;

    ReplyCallback(zeromq::ReplyCallback::ReceiveCallback callback);
    ReplyCallback() = delete;
    ReplyCallback(const ReplyCallback&) = delete;
    ReplyCallback(ReplyCallback&&) = delete;
    ReplyCallback& operator=(const ReplyCallback&) = delete;
    ReplyCallback& operator=(ReplyCallback&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_IMPLEMENTATION_HPP
