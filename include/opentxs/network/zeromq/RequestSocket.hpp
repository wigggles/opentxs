// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_REQUESTSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_REQUESTSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/CurveClient.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::RequestSocket::Factory;
%ignore opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>::Pimpl(opentxs::network::zeromq::RequestSocket const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>::operator opentxs::network::zeromq::RequestSocket&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>::operator const opentxs::network::zeromq::RequestSocket &;
%rename(assign) operator=(const opentxs::network::zeromq::RequestSocket&);
%rename(ZMQRequestSocket) opentxs::network::zeromq::RequestSocket;
%template(OTZMQRequestSocket) opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class RequestSocket : virtual public CurveClient
{
public:
    EXPORT static OTZMQRequestSocket Factory(const class Context& context);

    EXPORT virtual std::pair<
        opentxs::SendResult,
        opentxs::Pimpl<opentxs::network::zeromq::Message>>
    SendRequest(opentxs::Data& message) const = 0;
    EXPORT virtual std::pair<
        opentxs::SendResult,
        opentxs::Pimpl<opentxs::network::zeromq::Message>>
    SendRequest(const std::string& message) const = 0;
    EXPORT virtual std::pair<
        opentxs::SendResult,
        opentxs::Pimpl<opentxs::network::zeromq::Message>>
    SendRequest(opentxs::network::zeromq::Message& message) const = 0;
    EXPORT virtual bool SetSocksProxy(const std::string& proxy) const = 0;

    EXPORT virtual ~RequestSocket() = default;

protected:
    RequestSocket() = default;

private:
    friend OTZMQRequestSocket;

    virtual RequestSocket* clone() const = 0;

    RequestSocket(const RequestSocket&) = delete;
    RequestSocket(RequestSocket&&) = delete;
    RequestSocket& operator=(const RequestSocket&) = delete;
    RequestSocket& operator=(RequestSocket&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
