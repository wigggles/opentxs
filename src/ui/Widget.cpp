// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/Widget.hpp"   // IWYU pragma: associated

#include <algorithm>
#include <functional>
#include <thread>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"

#define OT_METHOD "opentxs::ui::implementation::Widget::"

namespace opentxs::ui::implementation
{
Widget::Widget(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const Identifier& id) noexcept
    : api_(api)
    , publisher_(publisher)
    , widget_id_(Identifier::Factory(id))
    , callbacks_()
    , listeners_()
    , cb_lock_()
    , cb_()
{
}

Widget::Widget(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher) noexcept
    : Widget(api, publisher, Identifier::Random())
{
}

void Widget::SetCallback(ui::Widget::Callback cb) const noexcept
{
    Lock lock(cb_lock_);
    cb_ = cb;
}

void Widget::setup_listeners(const ListenerDefinitions& definitions) noexcept
{
    for (const auto& [endpoint, functor] : definitions) {
        const auto* copy{functor};
        auto& nextCallback =
            callbacks_.emplace_back(network::zeromq::ListenCallback::Factory(
                [=](const network::zeromq::Message& message) -> void {
                    (*copy)(this, message);
                }));
        auto& socket = listeners_.emplace_back(
            api_.ZeroMQ().SubscribeSocket(nextCallback.get()));
        const auto listening = socket->Start(endpoint);

        OT_ASSERT(listening)
    }
}

void Widget::UpdateNotify() const noexcept
{
    LogTrace(OT_METHOD)(__FUNCTION__)(": Widget ")(widget_id_)(" updated.")
        .Flush();
    publisher_.Send(widget_id_->str());
    Lock lock(cb_lock_);

    if (cb_) {
        std::thread thread{[=]() -> void { cb_(); }};
        thread.detach();
    }
}

OTIdentifier Widget::WidgetID() const noexcept
{
    return Identifier::Factory(widget_id_);
}
}  // namespace opentxs::ui::implementation
