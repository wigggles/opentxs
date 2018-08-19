// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_PUBLISHSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_PUBLISHSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/CurveServer.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::PublishSocket::Factory;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PublishSocket>::Pimpl(opentxs::network::zeromq::PublishSocket const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::PublishSocket>::operator opentxs::network::zeromq::PublishSocket&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PublishSocket>::operator const opentxs::network::zeromq::PublishSocket &;
%rename(assign) operator=(const opentxs::network::zeromq::PublishSocket&);
%rename(ZMQPublishSocket) opentxs::network::zeromq::PublishSocket;
%template(OTZMQPublishSocket) opentxs::Pimpl<opentxs::network::zeromq::PublishSocket>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class PublishSocket : virtual public CurveServer
{
public:
    EXPORT static Pimpl<opentxs::network::zeromq::PublishSocket> Factory(
        const opentxs::network::zeromq::Context& context);

    EXPORT virtual bool Publish(const std::string& data) const = 0;
    EXPORT virtual bool Publish(const opentxs::Data& data) const = 0;
    EXPORT virtual bool Publish(network::zeromq::Message& data) const = 0;

    EXPORT virtual ~PublishSocket() = default;

protected:
    EXPORT PublishSocket() = default;

private:
    friend OTZMQPublishSocket;

    virtual PublishSocket* clone() const = 0;

    PublishSocket(const PublishSocket&) = delete;
    PublishSocket(PublishSocket&&) = default;
    PublishSocket& operator=(const PublishSocket&) = delete;
    PublishSocket& operator=(PublishSocket&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
