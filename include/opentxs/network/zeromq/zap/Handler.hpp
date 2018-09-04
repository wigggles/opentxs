// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_ZAP_HANDLER_HPP
#define OPENTXS_NETWORK_ZEROMQ_ZAP_HANDLER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/zap/ZAP.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Handler : virtual public zeromq::ReplySocket
{
public:
    EXPORT static OTZMQZAPHandler Factory(
        const zeromq::Context& context,
        const Callback& callback);

    EXPORT virtual ~Handler() = default;

protected:
    EXPORT Handler() = default;

private:
    friend OTZMQZAPHandler;

    virtual Handler* clone() const = 0;

    Handler(const Handler&) = delete;
    Handler(Handler&&) = default;
    Handler& operator=(const Handler&) = delete;
    Handler& operator=(Handler&&) = default;
};
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
