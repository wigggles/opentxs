// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_PULLSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_PULLSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/CurveServer.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::PullSocket::Factory;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PullSocket>::Pimpl(opentxs::network::zeromq::PullSocket const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::PullSocket>::operator opentxs::network::zeromq::PullSocket&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PullSocket>::operator const opentxs::network::zeromq::PullSocket &;
%rename(assign) operator=(const opentxs::network::zeromq::PullSocket&);
%rename(ZMQPullSocket) opentxs::network::zeromq::PullSocket;
%template(OTZMQPullSocket) opentxs::Pimpl<opentxs::network::zeromq::PullSocket>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class PullSocket : virtual public CurveServer
{
public:
    EXPORT static OTZMQPullSocket Factory(
        const class Context& context,
        const bool client,
        const ListenCallback& callback);
    EXPORT static OTZMQPullSocket Factory(
        const class Context& context,
        const bool client);

    EXPORT virtual ~PullSocket() = default;

protected:
    PullSocket() = default;

private:
    friend OTZMQPullSocket;

    virtual PullSocket* clone() const = 0;

    PullSocket(const PullSocket&) = delete;
    PullSocket(PullSocket&&) = default;
    PullSocket& operator=(const PullSocket&) = delete;
    PullSocket& operator=(PullSocket&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
