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

#ifndef OPENTXS_UI_WIDGET_IMPLEMENTATION_HPP
#define OPENTXS_UI_WIDGET_IMPLEMENTATION_HPP

#include "Internal.hpp"

#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/RequestSocket.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
class Widget : virtual public opentxs::ui::Widget
{
public:
    class MessageFunctor
    {
    public:
        virtual void operator()(Widget* object, const network::zeromq::Message&)
            const = 0;

    protected:
        MessageFunctor() = default;

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
            const override
        {
            auto real = dynamic_cast<T*>(object);

            OT_ASSERT(nullptr != real)

            (real->*callback_)(message);
        }

        MessageProcessor(Function callback)
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

    OTIdentifier WidgetID() const override;

    virtual ~Widget() = default;

protected:
    using ListenerDefinition = std::pair<std::string, MessageFunctor*>;
    using ListenerDefinitions = std::vector<ListenerDefinition>;

    const network::zeromq::Context& zmq_;
    const network::zeromq::PublishSocket& publisher_;

    void setup_listeners(const ListenerDefinitions& definitions);
    void UpdateNotify() const;

    Widget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& id);
    Widget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher);

private:
    const OTIdentifier widget_id_;
    std::vector<OTZMQListenCallback> callbacks_;
    std::vector<OTZMQSubscribeSocket> listeners_;

    Widget() = delete;
    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget& operator=(Widget&&) = delete;
};  // namespace opentxs::ui::implementation
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_WIDGET_IMPLEMENTATION_HPP
