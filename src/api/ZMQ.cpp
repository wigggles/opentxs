// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Log.hpp"

#include "ZMQ.hpp"

namespace opentxs::api::implementation
{
ZMQ::ZMQ(
    const opentxs::network::zeromq::Context& zmq,
    const int instance) noexcept
    : zmq_context_(zmq)
    , instance_(instance)
    , endpoints_p_(opentxs::Factory::Endpoints(zmq_context_, instance_))
    , endpoints_(*endpoints_p_)
    , shutdown_sender_(zmq, endpoints_.Shutdown())
{
    OT_ASSERT(endpoints_p_);
}
}  // namespace opentxs::api::implementation
