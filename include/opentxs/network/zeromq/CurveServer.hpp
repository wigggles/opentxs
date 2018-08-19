// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_CURVESERVER_HPP
#define OPENTXS_NETWORK_ZEROMQ_CURVESERVER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/Socket.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::CurveServer::SetPrivateKey;
%interface(opentxs::network::zeromq::CurveServer);
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class CurveServer : virtual public Socket
{
public:
    EXPORT virtual bool SetPrivateKey(const OTPassword& key) const = 0;

    EXPORT virtual ~CurveServer() = default;

protected:
    EXPORT CurveServer() = default;

private:
    CurveServer(const CurveServer&) = delete;
    CurveServer(CurveServer&&) = default;
    CurveServer& operator=(const CurveServer&) = delete;
    CurveServer& operator=(CurveServer&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

#endif
