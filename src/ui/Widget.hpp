// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
template <typename T>
T extract_custom(const CustomData& custom, const std::size_t index = 0) noexcept
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
        MessageFunctor& operator=(const MessageFunctor&) = delete;
        MessageFunctor& operator=(MessageFunctor&&) = delete;
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
        MessageProcessor& operator=(MessageProcessor&&) = default;

    private:
        Function callback_;

        MessageProcessor() = delete;
        MessageProcessor(const MessageProcessor&) = delete;
        MessageProcessor& operator=(const MessageProcessor&) = delete;
    };

    void SetCallback(ui::Widget::Callback cb) const noexcept final;
    OTIdentifier WidgetID() const noexcept override;

    ~Widget() override = default;

protected:
    using ListenerDefinition = std::pair<std::string, MessageFunctor*>;
    using ListenerDefinitions = std::vector<ListenerDefinition>;

    const api::client::internal::Manager& api_;
    const network::zeromq::socket::Publish& publisher_;
    const OTIdentifier widget_id_;

    void setup_listeners(const ListenerDefinitions& definitions) noexcept;
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
    Widget& operator=(const Widget&) = delete;
    Widget& operator=(Widget&&) = delete;
};  // namespace opentxs::ui::implementation
}  // namespace opentxs::ui::implementation
