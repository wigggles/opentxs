/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/stdafx.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "Widget.hpp"

#define OT_METHOD "opentxs::ui::implementation::Widget"

namespace opentxs::ui::implementation
{
Widget::Widget(const network::zeromq::Context& zmq, const Identifier& id)
    : zmq_(zmq)
    , widget_id_(Identifier::Factory(id))
    , update_socket_(opentxs::network::zeromq::RequestSocket::Factory(zmq))
{
    update_socket_->Start(
        opentxs::network::zeromq::Socket::WidgetUpdateCollectorEndpoint);
}

Widget::Widget(const network::zeromq::Context& zmq)
    : Widget(zmq, Identifier::Random())
{
}

void Widget::UpdateNotify() const
{
    auto id(widget_id_->str());
    otWarn << OT_METHOD << __FUNCTION__ << ": widget " << id << " updated"
           << std::endl;
    update_socket_->SendRequest(id);
}

OTIdentifier Widget::WidgetID() const
{
    return Identifier::Factory(widget_id_);
}
}  // namespace opentxs::ui::implementation
