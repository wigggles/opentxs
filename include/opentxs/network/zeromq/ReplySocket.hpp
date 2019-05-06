// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_REPLYSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_REPLYSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/CurveServer.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::ReplySocket::Factory;
%ignore opentxs::Pimpl<opentxs::network::zeromq::ReplySocket>::Pimpl(opentxs::network::zeromq::ReplySocket const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::ReplySocket>::operator opentxs::network::zeromq::ReplySocket&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::ReplySocket>::operator const opentxs::network::zeromq::ReplySocket &;
%rename(assign) operator=(const opentxs::network::zeromq::ReplySocket&);
%rename(ZMQReplySocket) opentxs::network::zeromq::ReplySocket;
%template(OTZMQReplySocket) opentxs::Pimpl<opentxs::network::zeromq::ReplySocket>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class ReplySocket : virtual public CurveServer
{
public:
    EXPORT static OTZMQReplySocket Factory(
        const class Context& context,
        const Socket::Direction direction,
        const ReplyCallback& callback);

    EXPORT virtual ~ReplySocket() = default;

protected:
    EXPORT ReplySocket() = default;

private:
    friend OTZMQReplySocket;

    virtual ReplySocket* clone() const = 0;

    ReplySocket(const ReplySocket&) = delete;
    ReplySocket(ReplySocket&&) = delete;
    ReplySocket& operator=(const ReplySocket&) = delete;
    ReplySocket& operator=(ReplySocket&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
