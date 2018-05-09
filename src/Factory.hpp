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

#ifndef OPENTXS_FACTORY_HPP
#define OPENTXS_FACTORY_HPP

namespace opentxs
{
class Factory
{
public:
    static ui::ActivitySummary* ActivitySummary(
        const network::zeromq::Context& zmq,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const Flag& running,
        const Identifier& nymID);
    static ui::ActivitySummaryItem* ActivitySummaryItem(
        const ui::implementation::ActivitySummaryParent& parent,
        const network::zeromq::Context& zmq,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const Flag& running,
        const Identifier& nymID,
        const Identifier& threadID);
    static ui::ActivityThread* ActivityThread(
        const network::zeromq::Context& zmq,
        const api::client::Sync& sync,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const Identifier& nymID,
        const Identifier& threadID);
    static api::Api* Api(
        const Flag& running,
        const api::Activity& activity,
        const api::Settings& config,
        const api::ContactManager& contacts,
        const api::Crypto& crypto,
        const api::Identity& identity,
        const api::storage::Storage& storage,
        const api::client::Wallet& wallet,
        const api::network::ZMQ& zmq);
    static api::client::Cash* Cash();
    static ui::ContactList* ContactList(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const Identifier& nymID);
    static ui::ContactListItem* ContactListItem(
        const ui::implementation::ContactListParent& parent,
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const Identifier& id,
        const std::string& name);
    static api::client::Issuer* Issuer(
        const api::client::Wallet& wallet,
        const Identifier& nymID,
        const proto::Issuer& serialized);
    static api::client::Issuer* Issuer(
        const api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& issuerID);
    static ui::ActivityThreadItem* MailItem(
        const ui::implementation::ActivityThreadParent& parent,
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const ui::implementation::ActivityThreadID& id,
        const Identifier& nymID,
        const api::Activity& activity,
        const std::chrono::system_clock::time_point& time,
        const std::string& text,
        const bool loading,
        const bool pending);
    static ui::ActivityThreadItem* MailItem(
        const ui::implementation::ActivityThreadParent& parent,
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const ui::implementation::ActivityThreadID& id,
        const Identifier& nymID,
        const api::Activity& activity,
        const std::chrono::system_clock::time_point& time);
    static ui::MessagableList* MessagableList(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const Identifier& nymID);
    static api::client::Pair* Pair(
        const Flag& running,
        const api::client::Sync& sync,
        const api::client::ServerAction& action,
        const api::client::Wallet& wallet,
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const network::zeromq::Context& context);
    static ui::ActivityThreadItem* PaymentItem(
        const ui::implementation::ActivityThreadParent& parent,
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const ui::implementation::ActivityThreadID& id,
        const Identifier& nymID,
        const api::Activity& activity,
        const std::chrono::system_clock::time_point& time);
    static ui::PayableList* PayableList(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const Identifier& nymID,
        const proto::ContactItemType& currency);
    static ui::PayableListItem* PayableListItem(
        const ui::implementation::ContactListParent& parent,
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const Identifier& id,
        const std::string& name,
        const std::string& paymentcode,
        const proto::ContactItemType& currency);
    static api::client::ServerAction* ServerAction(
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const ContextLockCallback& lockCallback);
    static api::client::Sync* Sync(
        const Flag& running,
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const api::ContactManager& contacts,
        const api::Settings& config,
        const api::Api& api,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const api::crypto::Encode& encoding,
        const network::zeromq::Context& zmq,
        const ContextLockCallback& lockCallback);
    static api::UI* UI(
        const network::zeromq::Context& zmq,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const Flag& running);
    static api::client::Wallet* Wallet(api::Native& ot);
    static api::client::Workflow* Workflow(
        const api::Activity& activity,
        const api::ContactManager& contact,
        const api::storage::Storage& storage);
};
}  // namespace opentxs
#endif  // OPENTXS_FACTORY_HPP
