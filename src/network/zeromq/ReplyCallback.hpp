// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_IMPLEMENTATION_HPP
#define OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_IMPLEMENTATION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/ReplyCallback.hpp"

namespace opentxs::network::zeromq::implementation
{
class ReplyCallback : virtual public zeromq::ReplyCallback
{
public:
    OTZMQMessage Process(const zeromq::Message& message) const override;

    ~ReplyCallback();

private:
    friend zeromq::ReplyCallback;

    const zeromq::ReplyCallback::ReceiveCallback callback_;

    ReplyCallback* clone() const override;

    ReplyCallback(zeromq::ReplyCallback::ReceiveCallback callback);
    ReplyCallback() = delete;
    ReplyCallback(const ReplyCallback&) = delete;
    ReplyCallback(ReplyCallback&&) = delete;
    ReplyCallback& operator=(const ReplyCallback&) = delete;
    ReplyCallback& operator=(ReplyCallback&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_IMPLEMENTATION_HPP
