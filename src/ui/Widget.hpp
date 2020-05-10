// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "internal/ui/UI.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace client
}  // namespace api

namespace network
{
namespace zeromq
{
class Message;

namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::ui::implementation
{
template <typename T>
auto extract_custom(
    const CustomData& custom,
    const std::size_t index = 0) noexcept -> T
{
    OT_ASSERT((index + 1) <= custom.size())

    std::unique_ptr<const T> output{static_cast<const T*>(custom.at(index))};

    OT_ASSERT(output)

    return *output;
}

class Widget : virtual public opentxs::ui::Widget
{
public:
    class MessageFunctor
    {
    public:
        virtual void operator()(Widget* object, const network::zeromq::Message&)
            const noexcept = 0;

        virtual ~MessageFunctor() = default;

    protected:
        MessageFunctor() noexcept = default;

    private:
        MessageFunctor(const MessageFunctor&) = delete;
        MessageFunctor(MessageFunctor&&) = delete;
        auto operator=(const MessageFunctor&) -> MessageFunctor& = delete;
        auto operator=(MessageFunctor &&) -> MessageFunctor& = delete;
    };

    template <typename T>
    class MessageProcessor : virtual public MessageFunctor
    {
    public:
        typedef void (T::*Function)(const network::zeromq::Message&);

        void operator()(Widget* object, const network::zeromq::Message& message)
            const noexcept final
        {
            auto real = dynamic_cast<T*>(object);

            OT_ASSERT(nullptr != real)

            (real->*callback_)(message);
        }

        MessageProcessor(Function callback) noexcept
            : callback_(callback)
        {
        }
        MessageProcessor(MessageProcessor&&) = default;
        auto operator=(MessageProcessor &&) -> MessageProcessor& = default;

    private:
        Function callback_;

        MessageProcessor() = delete;
        MessageProcessor(const MessageProcessor&) = delete;
        auto operator=(const MessageProcessor&) -> MessageProcessor& = delete;
    };

    void SetCallback(ui::Widget::Callback cb) const noexcept final;
    auto WidgetID() const noexcept -> OTIdentifier override;

    ~Widget() override = default;

protected:
    using ListenerDefinition = std::pair<std::string, MessageFunctor*>;
    using ListenerDefinitions = std::vector<ListenerDefinition>;

    const api::client::internal::Manager& api_;
    const network::zeromq::socket::Publish& publisher_;
    const OTIdentifier widget_id_;

    virtual void setup_listeners(
        const ListenerDefinitions& definitions) noexcept;
    void UpdateNotify() const noexcept;

    Widget(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const Identifier& id) noexcept;
    Widget(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher) noexcept;

private:
    std::vector<OTZMQListenCallback> callbacks_;
    std::vector<OTZMQSubscribeSocket> listeners_;
    mutable std::mutex cb_lock_;
    mutable ui::Widget::Callback cb_;

    Widget() = delete;
    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    auto operator=(const Widget&) -> Widget& = delete;
    auto operator=(Widget &&) -> Widget& = delete;
};  // namespace opentxs::ui::implementation
}  // namespace opentxs::ui::implementation
