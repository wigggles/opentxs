// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/network/zeromq/Socket.hpp"
#include "opentxs/network/zeromq/PairEventCallback.hpp"

#include "PairEventListener.hpp"

#include "opentxs/core/Log.hpp"

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::implementation::PairEventListener::"

namespace opentxs::network::zeromq::implementation
{
PairEventListener::PairEventListener(
    const zeromq::Context& context,
    const zeromq::PairEventCallback& callback)
    : ot_super(context, callback)
{
    Lock lock(lock_);
    const bool started = start_client(lock, zeromq::Socket::PairEventEndpoint);

    OT_ASSERT(started)

    otInfo << OT_METHOD << __FUNCTION__ << ": Subscriber listening on "
           << zeromq::Socket::PairEventEndpoint << std::endl;
}

PairEventListener* PairEventListener::clone() const
{
    return new PairEventListener(
        context_, dynamic_cast<const zeromq::PairEventCallback&>(callback_));
}

PairEventListener::~PairEventListener() {}
}  // namespace opentxs::network::zeromq::implementation
