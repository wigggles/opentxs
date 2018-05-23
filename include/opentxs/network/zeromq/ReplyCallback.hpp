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

#ifndef OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_HPP
#define OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_HPP

#include "opentxs/Forward.hpp"

#include <functional>

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::ReplyCallback::Factory;
%ignore opentxs::Pimpl<opentxs::network::zeromq::ReplyCallback>::Pimpl(opentxs::network::zeromq::ReplyCallback const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::ReplyCallback>::operator opentxs::network::zeromq::ReplyCallback&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::ReplyCallback>::operator const opentxs::network::zeromq::ReplyCallback &;
%rename(assign) operator=(const opentxs::network::zeromq::ReplyCallback&);
%rename(ZMQReplyCallback) opentxs::network::zeromq::ReplyCallback;
%template(OTZMQReplyCallback) opentxs::Pimpl<opentxs::network::zeromq::ReplyCallback>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class ReplyCallback
{
public:
    using ReceiveCallback =
        std::function<OTZMQMultipartMessage(const MultipartMessage&)>;

    EXPORT static OTZMQReplyCallback Factory(ReceiveCallback callback);

    EXPORT virtual Pimpl<opentxs::network::zeromq::MultipartMessage> Process(
        const MultipartMessage& message) const = 0;

    EXPORT virtual ~ReplyCallback() = default;

protected:
    ReplyCallback() = default;

private:
    friend OTZMQReplyCallback;

    virtual ReplyCallback* clone() const = 0;

    ReplyCallback(const ReplyCallback&) = delete;
    ReplyCallback(ReplyCallback&&) = default;
    ReplyCallback& operator=(const ReplyCallback&) = delete;
    ReplyCallback& operator=(ReplyCallback&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_HPP
