// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"

#include "internal/api/Api.hpp"

#include "Pipeline.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::Pipeline>;

//#define OT_METHOD
//"opentxs::network::zeromq::socket::implementation::Pipeline::"

namespace opentxs
{
opentxs::network::zeromq::Pipeline* Factory::Pipeline(
    const api::internal::Core& api,
    const network::zeromq::Context& context,
    std::function<void(network::zeromq::Message&)> callback)
{
    return new opentxs::network::zeromq::socket::implementation::Pipeline(
        api, context, callback);
}
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
Pipeline::Pipeline(
    const api::internal::Core& api,
    const zeromq::Context& context,
    std::function<void(zeromq::Message&)> callback) noexcept
    : sender_(context.PushSocket(Socket::Direction::Bind))
    , callback_(ListenCallback::Factory(callback))
    , receiver_(context.SubscribeSocket(callback_))
{
    const auto endpoint = std::string("inproc://opentxs/") +
                          api.Crypto().Encode().Nonce(32)->Get();
    auto started = receiver_->Start(endpoint);
    started &= sender_->Start(endpoint);

    OT_ASSERT(started);
}

bool Pipeline::Close() const noexcept
{
    return sender_->Close() && receiver_->Close();
}

Pipeline::~Pipeline() { Close(); }
}  // namespace opentxs::network::zeromq::socket::implementation
