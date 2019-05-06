// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_DEALERSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_DEALERSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/CurveClient.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::DealerSocket::Factory;
%ignore opentxs::Pimpl<opentxs::network::zeromq::DealerSocket>::Pimpl(opentxs::network::zeromq::DealerSocket const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::DealerSocket>::operator opentxs::network::zeromq::DealerSocket&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::DealerSocket>::operator const opentxs::network::zeromq::DealerSocket &;
%rename(assign) operator=(const opentxs::network::zeromq::DealerSocket&);
%rename(ZMQDealerSocket) opentxs::network::zeromq::DealerSocket;
%template(OTZMQDealerSocket) opentxs::Pimpl<opentxs::network::zeromq::DealerSocket>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class DealerSocket : virtual public CurveClient
{
public:
    EXPORT static OTZMQDealerSocket Factory(
        const class Context& context,
        const Socket::Direction direction,
        const ListenCallback& callback);

    EXPORT virtual bool Send(opentxs::Data& message) const = 0;
    EXPORT virtual bool Send(const std::string& message) const = 0;
    EXPORT virtual bool Send(
        opentxs::network::zeromq::Message& message) const = 0;
    EXPORT virtual bool SetSocksProxy(const std::string& proxy) const = 0;

    EXPORT virtual ~DealerSocket() = default;

protected:
    DealerSocket() = default;

private:
    friend OTZMQDealerSocket;

    virtual DealerSocket* clone() const = 0;

    DealerSocket(const DealerSocket&) = delete;
    DealerSocket(DealerSocket&&) = delete;
    DealerSocket& operator=(const DealerSocket&) = delete;
    DealerSocket& operator=(DealerSocket&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
