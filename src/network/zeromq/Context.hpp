// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::network::zeromq::implementation
{
class Context final : virtual public zeromq::Context
{
public:
    operator void*() const noexcept final;

    std::string BuildEndpoint(
        const std::string& path,
        const int instance,
        const int version) const noexcept final;
    std::string BuildEndpoint(
        const std::string& path,
        const int instance,
        const int version,
        const std::string& suffix) const noexcept final;
    OTZMQDealerSocket DealerSocket(
        const ListenCallback& callback,
        const socket::Socket::Direction direction) const noexcept final;
    OTZMQFrame Frame(const void* input, const std::size_t size) const
        noexcept final;
    OTZMQMessage Message() const noexcept final;
    OTZMQMessage Message(const ProtobufType& input) const noexcept final;
    OTZMQMessage Message(const network::zeromq::Message& input) const
        noexcept final
    {
        return OTZMQMessage(input);
    }
    OTZMQMessage Message(const void* input, const std::size_t size) const
        noexcept final;
    OTZMQSubscribeSocket PairEventListener(
        const PairEventCallback& callback,
        const int instance) const noexcept final;
    OTZMQPairSocket PairSocket(
        const opentxs::network::zeromq::ListenCallback& callback) const
        noexcept final;
    OTZMQPairSocket PairSocket(
        const opentxs::network::zeromq::ListenCallback& callback,
        const opentxs::network::zeromq::socket::Pair& peer) const
        noexcept final;
    OTZMQPairSocket PairSocket(
        const opentxs::network::zeromq::ListenCallback& callback,
        const std::string& endpoint) const noexcept final;
    OTZMQPipeline Pipeline(
        const api::internal::Core& api,
        std::function<void(zeromq::Message&)> callback) const noexcept final;
    OTZMQProxy Proxy(
        network::zeromq::socket::Socket& frontend,
        network::zeromq::socket::Socket& backend) const noexcept final;
    OTZMQPublishSocket PublishSocket() const noexcept final;
    OTZMQPullSocket PullSocket(const socket::Socket::Direction direction) const
        noexcept final;
    OTZMQPullSocket PullSocket(
        const ListenCallback& callback,
        const socket::Socket::Direction direction) const noexcept final;
    OTZMQPushSocket PushSocket(const socket::Socket::Direction direction) const
        noexcept final;
    OTZMQMessage ReplyMessage(const zeromq::Message& request) const
        noexcept final;
    OTZMQReplySocket ReplySocket(
        const ReplyCallback& callback,
        const socket::Socket::Direction direction) const noexcept final;
    OTZMQRequestSocket RequestSocket() const noexcept final;
    OTZMQRouterSocket RouterSocket(
        const ListenCallback& callback,
        const socket::Socket::Direction direction) const noexcept final;
    OTZMQSubscribeSocket SubscribeSocket(const ListenCallback& callback) const
        noexcept final;

    ~Context();

private:
    friend opentxs::Factory;

    void* context_{nullptr};

    Context* clone() const noexcept final { return new Context; }

    Context() noexcept;
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
