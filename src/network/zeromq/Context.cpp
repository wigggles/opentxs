// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/Ed25519.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Pair.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/Proxy.hpp"

#include "PairEventListener.hpp"

#include <zmq.h>

#include <vector>

#include "Context.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::Context>;

#define INPROC_PREFIX "inproc://opentxs/"
#define PATH_SEPERATOR "/"
#define OT_METHOD "opentxs::Context::"

namespace opentxs
{
network::zeromq::Context* Factory::ZMQContext()
{
    using ReturnType = network::zeromq::implementation::Context;

    return new ReturnType;
}
}  // namespace opentxs

namespace opentxs::network::zeromq
{
auto Context::RawToZ85(
    const ReadView input,
    const AllocateOutput destination) noexcept -> bool
{
    if (0 != input.size() % 4) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid input size.").Flush();

        return false;
    }

    if (false == bool(destination)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator.")
            .Flush();

        return false;
    }

    const auto target = std::size_t{input.size() + input.size() / 4 + 1};
    auto out = destination(target);

    if (false == out.valid(target)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to allocate output")
            .Flush();

        return false;
    }

    return nullptr != ::zmq_z85_encode(
                          out.as<char>(),
                          reinterpret_cast<const std::uint8_t*>(input.data()),
                          input.size());
}

auto Context::Z85ToRaw(
    const ReadView input,
    const AllocateOutput destination) noexcept -> bool
{
    if (0 != input.size() % 5) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid input size.").Flush();

        return false;
    }

    if (false == bool(destination)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator.")
            .Flush();

        return false;
    }

    const auto target = std::size_t{input.size() * 4 / 5};
    auto out = destination(target);

    if (false == out.valid(target)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to allocate output")
            .Flush();

        return false;
    }

    return ::zmq_z85_decode(out.as<std::uint8_t>(), input.data());
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
Context::Context() noexcept
    : context_(::zmq_ctx_new())
{
    OT_ASSERT(nullptr != context_);
    OT_ASSERT(1 == ::zmq_has("curve"));

    auto init = ::zmq_ctx_set(context_, ZMQ_MAX_SOCKETS, 16384);

    OT_ASSERT(0 == init);
}

Context::operator void*() const noexcept
{
    OT_ASSERT(nullptr != context_)

    return context_;
}

std::string Context::BuildEndpoint(
    const std::string& path,
    const int instance,
    const int version) const noexcept
{
    return std::string(INPROC_PREFIX) + std::to_string(instance) +
           PATH_SEPERATOR + path + PATH_SEPERATOR + std::to_string(version);
}

std::string Context::BuildEndpoint(
    const std::string& path,
    const int instance,
    const int version,
    const std::string& suffix) const noexcept
{
    return BuildEndpoint(path, instance, version) + PATH_SEPERATOR + suffix;
}

OTZMQDealerSocket Context::DealerSocket(
    const ListenCallback& callback,
    const socket::Socket::Direction direction) const noexcept
{
    return OTZMQDealerSocket{
        Factory::DealerSocket(*this, static_cast<bool>(direction), callback)};
}

OTZMQFrame Context::Frame(const void* input, const std::size_t size) const
    noexcept
{
    return OTZMQFrame{Factory::ZMQFrame(input, size)};
}

OTZMQMessage Context::Message() const noexcept
{
    return OTZMQMessage{Factory::ZMQMessage()};
}

OTZMQMessage Context::Message(const ProtobufType& input) const noexcept
{
    return OTZMQMessage{Factory::ZMQMessage(input)};
}

OTZMQMessage Context::Message(const void* input, const std::size_t size) const
    noexcept
{
    return OTZMQMessage{Factory::ZMQMessage(input, size)};
}

OTZMQSubscribeSocket Context::PairEventListener(
    const PairEventCallback& callback,
    const int instance) const noexcept
{
    return OTZMQSubscribeSocket(
        new class PairEventListener(*this, callback, instance));
}

OTZMQPairSocket Context::PairSocket(
    const opentxs::network::zeromq::ListenCallback& callback) const noexcept
{
    return OTZMQPairSocket{Factory::PairSocket(*this, callback, true)};
}

OTZMQPairSocket Context::PairSocket(
    const opentxs::network::zeromq::ListenCallback& callback,
    const opentxs::network::zeromq::socket::Pair& peer) const noexcept
{
    return OTZMQPairSocket{Factory::PairSocket(callback, peer, true)};
}

OTZMQPairSocket Context::PairSocket(
    const opentxs::network::zeromq::ListenCallback& callback,
    const std::string& endpoint) const noexcept
{
    return OTZMQPairSocket{Factory::PairSocket(*this, callback, endpoint)};
}

OTZMQPipeline Context::Pipeline(
    const api::internal::Core& api,
    std::function<void(zeromq::Message&)> callback) const noexcept
{
    return OTZMQPipeline{opentxs::Factory::Pipeline(api, *this, callback)};
}

OTZMQProxy Context::Proxy(
    network::zeromq::socket::Socket& frontend,
    network::zeromq::socket::Socket& backend) const noexcept
{
    return opentxs::network::zeromq::Proxy::Factory(*this, frontend, backend);
}

OTZMQPublishSocket Context::PublishSocket() const noexcept
{
    return OTZMQPublishSocket{Factory::PublishSocket(*this)};
}

OTZMQPullSocket Context::PullSocket(
    const socket::Socket::Direction direction) const noexcept
{
    return OTZMQPullSocket{
        Factory::PullSocket(*this, static_cast<bool>(direction))};
}

OTZMQPullSocket Context::PullSocket(
    const ListenCallback& callback,
    const socket::Socket::Direction direction) const noexcept
{
    return OTZMQPullSocket{
        Factory::PullSocket(*this, static_cast<bool>(direction), callback)};
}

OTZMQPushSocket Context::PushSocket(
    const socket::Socket::Direction direction) const noexcept
{
    return OTZMQPushSocket{
        Factory::PushSocket(*this, static_cast<bool>(direction))};
}

OTZMQMessage Context::ReplyMessage(const zeromq::Message& request) const
    noexcept
{
    auto output = Message();

    if (0 < request.Header().size()) {
        for (const auto& frame : request.Header()) { output->AddFrame(frame); }

        output->AddFrame();
    }

    OT_ASSERT(0 == output->Body().size());

    return output;
}

OTZMQReplySocket Context::ReplySocket(
    const ReplyCallback& callback,
    const socket::Socket::Direction direction) const noexcept
{
    return OTZMQReplySocket{
        Factory::ReplySocket(*this, static_cast<bool>(direction), callback)};
}

OTZMQRequestSocket Context::RequestSocket() const noexcept
{
    return OTZMQRequestSocket{Factory::RequestSocket(*this)};
}

OTZMQRouterSocket Context::RouterSocket(
    const ListenCallback& callback,
    const socket::Socket::Direction direction) const noexcept
{
    return OTZMQRouterSocket{
        Factory::RouterSocket(*this, static_cast<bool>(direction), callback)};
}

OTZMQSubscribeSocket Context::SubscribeSocket(
    const ListenCallback& callback) const noexcept
{
    return OTZMQSubscribeSocket{Factory::SubscribeSocket(*this, callback)};
}

Context::~Context()
{
    if (nullptr != context_) { zmq_ctx_shutdown(context_); }
}
}  // namespace opentxs::network::zeromq::implementation
