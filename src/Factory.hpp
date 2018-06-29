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
    static ui::AccountActivity* AccountActivity(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Sync& sync,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const api::ContactManager& contact,
        const api::storage::Storage& storage,
        const Identifier& nymID,
        const Identifier& accountID);
    static ui::ActivitySummary* ActivitySummary(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const Flag& running,
        const Identifier& nymID);
    static ui::ActivitySummaryItem* ActivitySummaryItem(
        const ui::implementation::ActivitySummaryParent& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const Flag& running,
        const Identifier& nymID,
        const Identifier& threadID);
    static ui::ActivityThread* ActivityThread(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
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
    static ui::BalanceItem* BalanceItem(
        const ui::implementation::AccountActivityParent& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const api::client::Wallet& wallet,
        const proto::PaymentWorkflow& workflow,
        const proto::PaymentEvent& event,
        const Identifier& nymID,
        const Identifier& accountID);
    static api::Blockchain* Blockchain(
        const api::Activity& activity,
        const api::Crypto& crypto,
        const api::storage::Storage& storage,
        const api::client::Wallet& wallet);
    static api::client::Cash* Cash();
    static ui::ContactList* ContactList(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& nymID);
    static ui::ContactListItem* ContactListItem(
        const ui::implementation::ContactListParent& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& id,
        const std::string& name);
    static ui::Contact* ContactWidget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& contactID);
    static ui::ContactItem* ContactItemWidget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ContactSubsectionParent& parent,
        const ContactItem& item);
    static ui::ContactSection* ContactSectionWidget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ContactParent& parent,
        const ContactSection& section);
    static ui::ContactSubsection* ContactSubsectionWidget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ContactSectionParent& parent,
        const ContactGroup& group);
    static api::Identity* Identity(const api::client::Wallet& wallet);
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
        const network::zeromq::PublishSocket& publisher,
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
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ActivityThreadID& id,
        const Identifier& nymID,
        const api::Activity& activity,
        const std::chrono::system_clock::time_point& time);
    static ui::MessagableList* MessagableList(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
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
    static ui::PayableList* PayableList(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const Identifier& nymID,
        const proto::ContactItemType& currency);
    static ui::PayableListItem* PayableListItem(
        const ui::implementation::ContactListParent& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& id,
        const std::string& name,
        const std::string& paymentcode,
        const proto::ContactItemType& currency);
    static ui::ActivityThreadItem* PaymentItem(
        const ui::implementation::ActivityThreadParent& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ActivityThreadID& id,
        const Identifier& nymID,
        const api::Activity& activity,
        const std::chrono::system_clock::time_point& time);
    static ui::Profile* ProfileWidget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Wallet& wallet,
        const Identifier& nymID);
    static ui::ProfileItem* ProfileItemWidget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Wallet& wallet,
        const ui::implementation::ProfileSubsectionParent& parent,
        const ContactItem& item);
    static ui::ProfileSection* ProfileSectionWidget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Wallet& wallet,
        const ui::implementation::ProfileParent& parent,
        const ContactSection& section);
    static ui::ProfileSubsection* ProfileSubsectionWidget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Wallet& wallet,
        const ui::implementation::ProfileSectionParent& parent,
        const ContactGroup& group);
    static api::client::ServerAction* ServerAction(
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const ContextLockCallback& lockCallback);
    static api::storage::StorageInternal* Storage(
        const Flag& running,
        const StorageConfig& config,
        const String& primary,
        const bool migrate,
        const String& previous,
        const Digest& hash,
        const Random& random);
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
        const api::storage::Storage& storage,
        const network::zeromq::Context& zmq,
        const ContextLockCallback& lockCallback);
    static api::UI* UI(
        const network::zeromq::Context& zmq,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const api::storage::Storage& storage,
        const Flag& running);
    static api::client::Wallet* Wallet(
        const api::Native& ot,
        const network::zeromq::Context& zmq);
    static api::client::Workflow* Workflow(
        const api::Activity& activity,
        const api::ContactManager& contact,
        const api::storage::Storage& storage,
        const network::zeromq::Context& zmq);
};
}  // namespace opentxs
#endif  // OPENTXS_FACTORY_HPP
