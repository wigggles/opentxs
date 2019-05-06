// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_PUSHSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_PUSHSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/CurveClient.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::PushSocket::Factory;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PushSocket>::Pimpl(opentxs::network::zeromq::PushSocket const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::PushSocket>::operator opentxs::network::zeromq::PushSocket&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PushSocket>::operator const opentxs::network::zeromq::PushSocket &;
%rename(assign) operator=(const opentxs::network::zeromq::PushSocket&);
%rename(ZMQPushSocket) opentxs::network::zeromq::PushSocket;
%template(OTZMQPushSocket) opentxs::Pimpl<opentxs::network::zeromq::PushSocket>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class PushSocket : virtual public CurveClient
{
public:
    EXPORT static Pimpl<opentxs::network::zeromq::PushSocket> Factory(
        const opentxs::network::zeromq::Context& context,
        const Socket::Direction direction);

    EXPORT virtual bool Push(const std::string& data) const = 0;
    EXPORT virtual bool Push(const opentxs::Data& data) const = 0;
    EXPORT virtual bool Push(network::zeromq::Message& data) const = 0;

    EXPORT virtual ~PushSocket() = default;

protected:
    EXPORT PushSocket() = default;

private:
    friend OTZMQPushSocket;

    virtual PushSocket* clone() const = 0;

    PushSocket(const PushSocket&) = delete;
    PushSocket(PushSocket&&) = delete;
    PushSocket& operator=(const PushSocket&) = delete;
    PushSocket& operator=(PushSocket&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
