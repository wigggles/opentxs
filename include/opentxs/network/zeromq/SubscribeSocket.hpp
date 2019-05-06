// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SUBSCRIBESOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_SUBSCRIBESOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/CurveClient.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::SubscribeSocket::Factory;
%ignore opentxs::Pimpl<opentxs::network::zeromq::SubscribeSocket>::Pimpl(opentxs::network::zeromq::SubscribeSocket const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::SubscribeSocket>::operator opentxs::network::zeromq::SubscribeSocket&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::SubscribeSocket>::operator const opentxs::network::zeromq::SubscribeSocket &;
%rename(assign) operator=(const opentxs::network::zeromq::SubscribeSocket&);
%rename(ZMQSubscribeSocket) opentxs::network::zeromq::SubscribeSocket;
%template(OTZMQSubscribeSocket) opentxs::Pimpl<opentxs::network::zeromq::SubscribeSocket>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class SubscribeSocket : virtual public CurveClient
{
public:
    EXPORT static OTZMQSubscribeSocket Factory(
        const class Context& context,
        const ListenCallback& callback);

    EXPORT virtual bool SetSocksProxy(const std::string& proxy) const = 0;

    EXPORT virtual ~SubscribeSocket() = default;

protected:
    SubscribeSocket() = default;

private:
    friend OTZMQSubscribeSocket;

    virtual SubscribeSocket* clone() const = 0;

    SubscribeSocket(const SubscribeSocket&) = delete;
    SubscribeSocket(SubscribeSocket&&) = delete;
    SubscribeSocket& operator=(const SubscribeSocket&) = delete;
    SubscribeSocket& operator=(SubscribeSocket&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
