// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_PUSH_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_PUSH_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/curve/Client.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"
#include "opentxs/Proto.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Push>::Pimpl(opentxs::network::zeromq::socket::Push const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Push>::operator opentxs::network::zeromq::socket::Push&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Push>::operator const opentxs::network::zeromq::socket::Push &;
%rename(assign) operator=(const opentxs::network::zeromq::socket::Push&);
%rename(ZMQPush) opentxs::network::zeromq::socket::Push;
%template(OTZMQPushSocket) opentxs::Pimpl<opentxs::network::zeromq::socket::Push>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Push : virtual public curve::Client, virtual public Sender
{
public:
    OPENTXS_EXPORT ~Push() override = default;

protected:
    Push() noexcept = default;

private:
    friend OTZMQPushSocket;

    virtual Push* clone() const noexcept = 0;

    Push(const Push&) = delete;
    Push(Push&&) = delete;
    Push& operator=(const Push&) = delete;
    Push& operator=(Push&&) = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
