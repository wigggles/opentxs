// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_CURVECLIENT_HPP
#define OPENTXS_NETWORK_ZEROMQ_CURVECLIENT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/Socket.hpp"

#include <tuple>

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::CurveClient::SetServerPubkey;
%template(CurveKeypair) std::pair<std::string, std::string>;
%interface(opentxs::network::zeromq::CurveClient);
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class CurveClient : virtual public Socket
{
public:
    EXPORT static std::pair<std::string, std::string> RandomKeypair();

    EXPORT virtual bool SetKeysZ85(
        const std::string& serverPublic,
        const std::string& clientPrivate,
        const std::string& clientPublic) const = 0;
    EXPORT virtual bool SetServerPubkey(
        const ServerContract& contract) const = 0;
    EXPORT virtual bool SetServerPubkey(const Data& key) const = 0;

    EXPORT virtual ~CurveClient() = default;

protected:
    EXPORT CurveClient() = default;

private:
    CurveClient(const CurveClient&) = delete;
    CurveClient(CurveClient&&) = delete;
    CurveClient& operator=(const CurveClient&) = delete;
    CurveClient& operator=(CurveClient&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

#endif
