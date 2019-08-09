// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_MESSAGE_HPP
#define OPENTXS_NETWORK_ZEROMQ_MESSAGE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/Proto.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::Message>::Pimpl(opentxs::network::zeromq::Message const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::Message>::operator opentxs::network::zeromq::Message&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Message>::operator const opentxs::network::zeromq::Message &;
%ignore opentxs::network::zeromq::Message::at(const std::size_t) const;
%ignore opentxs::network::zeromq::Message::begin() const;
%ignore opentxs::network::zeromq::Message::end() const;
%rename(assign) operator=(const opentxs::network::zeromq::Message&);
%rename(ZMQMessage) opentxs::network::zeromq::Message;
%template(OTZMQMessage) opentxs::Pimpl<opentxs::network::zeromq::Message>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
using OTZMQMessage = Pimpl<network::zeromq::Message>;

namespace network
{
namespace zeromq
{
class Message
{
public:
    OPENTXS_EXPORT static Pimpl<Message> Factory();

    OPENTXS_EXPORT virtual auto at(const std::size_t index) const
        -> const Frame& = 0;
    OPENTXS_EXPORT virtual auto begin() const -> FrameIterator = 0;
    OPENTXS_EXPORT virtual auto Body() const -> const FrameSection = 0;
    OPENTXS_EXPORT virtual auto Body_at(const std::size_t index) const
        -> const Frame& = 0;
    OPENTXS_EXPORT virtual auto Body_begin() const -> FrameIterator = 0;
    OPENTXS_EXPORT virtual auto Body_end() const -> FrameIterator = 0;
    OPENTXS_EXPORT virtual auto end() const -> FrameIterator = 0;
    OPENTXS_EXPORT virtual auto Header() const -> const FrameSection = 0;
    OPENTXS_EXPORT virtual auto Header_at(const std::size_t index) const
        -> const Frame& = 0;
    OPENTXS_EXPORT virtual auto Header_begin() const -> FrameIterator = 0;
    OPENTXS_EXPORT virtual auto Header_end() const -> FrameIterator = 0;
    OPENTXS_EXPORT virtual auto size() const -> std::size_t = 0;

    OPENTXS_EXPORT virtual auto AddFrame() -> Frame& = 0;
    OPENTXS_EXPORT virtual auto AddFrame(const ProtobufType& input)
        -> Frame& = 0;
#ifndef SWIG
    template <
        typename Input,
        std::enable_if_t<
            std::is_pointer<decltype(std::declval<Input&>().data())>::value,
            int> = 0,
        std::enable_if_t<
            std::is_integral<decltype(std::declval<Input&>().size())>::value,
            int> = 0>
    OPENTXS_EXPORT auto AddFrame(const Input& input) -> Frame&
    {
        return AddFrame(input.data(), input.size());
    }
    template <
        typename Input,
        std::enable_if_t<std::is_trivially_copyable<Input>::value, int> = 0>
    OPENTXS_EXPORT auto AddFrame(const Input& input) -> Frame&
    {
        return AddFrame(&input, sizeof(input));
    }
    template <typename Input>
    OPENTXS_EXPORT auto AddFrame(const Pimpl<Input>& input) -> Frame&
    {
        return AddFrame(input.get());
    }
#endif
    OPENTXS_EXPORT virtual auto AddFrame(
        const void* input,
        const std::size_t size) -> Frame& = 0;
    OPENTXS_EXPORT virtual auto at(const std::size_t index) -> Frame& = 0;
    OPENTXS_EXPORT virtual auto Body() -> FrameSection = 0;
    OPENTXS_EXPORT virtual auto EnsureDelimiter() -> void = 0;
    OPENTXS_EXPORT virtual auto Header() -> FrameSection = 0;
    OPENTXS_EXPORT virtual auto PrependEmptyFrame() -> void = 0;
    OPENTXS_EXPORT virtual auto Replace(
        const std::size_t index,
        OTZMQFrame&& frame) -> Frame& = 0;

    OPENTXS_EXPORT virtual ~Message() = default;

protected:
    Message() = default;

private:
    friend OTZMQMessage;

#ifdef _WIN32
public:
#endif
    OPENTXS_EXPORT virtual auto clone() const -> Message* = 0;
#ifdef _WIN32
private:
#endif

    Message(const Message&) = delete;
    Message(Message&&) = default;
    Message& operator=(const Message&) = delete;
    Message& operator=(Message&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
