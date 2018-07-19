// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"

#include "Widget.hpp"

#define OT_METHOD "opentxs::ui::implementation::Widget::"

namespace opentxs::ui::implementation
{
Widget::Widget(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const Identifier& id)
    : zmq_(zmq)
    , publisher_(publisher)
    , widget_id_(Identifier::Factory(id))
    , callbacks_()
    , listeners_()
{
}

Widget::Widget(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher)
    : Widget(zmq, publisher, Identifier::Random())
{
}

void Widget::setup_listeners(const ListenerDefinitions& definitions)
{
    for (const auto& [endpoint, functor] : definitions) {
        const auto* copy{functor};
        auto& nextCallback =
            callbacks_.emplace_back(network::zeromq::ListenCallback::Factory(
                [=](const network::zeromq::Message& message) -> void {
                    (*copy)(this, message);
                }));
        auto& socket =
            listeners_.emplace_back(zmq_.SubscribeSocket(nextCallback.get()));
        const auto listening = socket->Start(endpoint);

        OT_ASSERT(listening)
    }
}

void Widget::UpdateNotify() const
{
    auto id(widget_id_->str());
    publisher_.Publish(id);
    otErr << OT_METHOD << __FUNCTION__ << ": widget " << id << " updated"
          << std::endl;
}

OTIdentifier Widget::WidgetID() const
{
    return Identifier::Factory(widget_id_);
}
}  // namespace opentxs::ui::implementation
