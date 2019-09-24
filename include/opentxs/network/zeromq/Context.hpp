// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_CONTEXT_HPP
#define OPENTXS_NETWORK_ZEROMQ_CONTEXT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/Proto.hpp"

#include <memory>
#include <string>

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::Pimpl(opentxs::network::zeromq::Context const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::operator opentxs::network::zeromq::Context&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::operator const opentxs::network::zeromq::Context &;
%ignore opentxs::network::zeromq::Context::operator void*() const;
%ignore opentxs::network::zeromq::Context::EncodePrivateZ85 const;
%ignore opentxs::network::zeromq::Context::Pipeline const;
%rename(assign) operator=(const opentxs::network::zeromq::Context&);
%rename(ZMQContext) opentxs::network::zeromq::Context;
%template(OTZMQContext) opentxs::Pimpl<opentxs::network::zeromq::Context>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context
{
public:
    EXPORT static std::string EncodePrivateZ85(
        const opentxs::crypto::key::Ed25519& key) noexcept;
    EXPORT static std::string RawToZ85(
        const void* input,
        const std::size_t size) noexcept;
    EXPORT static opentxs::Pimpl<opentxs::Data> Z85ToRaw(
        const void* input,
        const std::size_t size) noexcept;

    EXPORT virtual operator void*() const noexcept = 0;

    EXPORT virtual std::string BuildEndpoint(
        const std::string& path,
        const int instance,
        const int version) const noexcept = 0;
    EXPORT virtual std::string BuildEndpoint(
        const std::string& path,
        const int instance,
        const int version,
        const std::string& suffix) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Dealer> DealerSocket(
        const ListenCallback& callback,
        const socket::Socket::Direction direction) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::Message> Message() const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::Message> Message(
        const ProtobufType& input) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::Message> Message(
        const network::zeromq::Message& input) const noexcept = 0;
#ifndef SWIG
    template <
        typename Input,
        std::enable_if_t<
            std::is_pointer<decltype(std::declval<Input&>().data())>::value,
            int> = 0,
        std::enable_if_t<
            std::is_integral<decltype(std::declval<Input&>().size())>::value,
            int> = 0>
    EXPORT Pimpl<network::zeromq::Message> Message(const Input& input) const
        noexcept
    {
        return Message(input.data(), input.size());
    }
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    EXPORT Pimpl<network::zeromq::Message> Message(const Input& input) const
        noexcept
    {
        return Message(&input, sizeof(input));
    }
    template <typename Input>
    EXPORT Pimpl<network::zeromq::Message> Message(
        const Pimpl<Input>& input) const noexcept
    {
        return Message(input.get());
    }
#endif
    EXPORT virtual Pimpl<network::zeromq::Message> Message(
        const void* input,
        const std::size_t size) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Subscribe> PairEventListener(
        const PairEventCallback& callback,
        const int instance) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Pair> PairSocket(
        const ListenCallback& callback) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Pair> PairSocket(
        const ListenCallback& callback,
        const zeromq::socket::Pair& peer) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Pair> PairSocket(
        const ListenCallback& callback,
        const std::string& endpoint) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::Pipeline> Pipeline(
        const api::Core& api,
        std::function<void(zeromq::Message&)> callback) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::Proxy> Proxy(
        socket::Socket& frontend,
        socket::Socket& backend) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Publish> PublishSocket() const
        noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Pull> PullSocket(
        const socket::Socket::Direction direction) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Pull> PullSocket(
        const ListenCallback& callback,
        const socket::Socket::Direction direction) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Push> PushSocket(
        const socket::Socket::Direction direction) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::Message> ReplyMessage(
        const zeromq::Message& request) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Reply> ReplySocket(
        const ReplyCallback& callback,
        const socket::Socket::Direction direction) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Request> RequestSocket() const
        noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Router> RouterSocket(
        const ListenCallback& callback,
        const socket::Socket::Direction direction) const noexcept = 0;
    EXPORT virtual Pimpl<network::zeromq::socket::Subscribe> SubscribeSocket(
        const ListenCallback& callback) const noexcept = 0;

    EXPORT virtual ~Context() = default;

protected:
    Context() noexcept = default;

private:
    friend OTZMQContext;

    EXPORT virtual Context* clone() const noexcept = 0;

    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
