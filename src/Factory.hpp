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
    static ui::AccountSummary* AccountSummary(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Wallet& wallet,
        const api::network::ZMQ& connection,
        const api::storage::Storage& storage,
        const api::ContactManager& contact,
        const Identifier& nymID,
        const proto::ContactItemType currency);
    static ui::AccountSummaryItem* AccountSummaryItem(
        const ui::implementation::IssuerItemParent& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Wallet& wallet,
        const api::storage::Storage& storage,
        const api::ContactManager& contact,
        const ui::implementation::IssuerItemRowID& id);
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
    static api::Crypto* Crypto(const api::Native& native);
    static api::crypto::Encode* Encode(crypto::EncodingProvider& base58);
    static api::crypto::Hash* Hash(
        api::crypto::Encode& encode,
        crypto::HashingProvider& ssl,
        crypto::HashingProvider& sodium
#if OT_CRYPTO_USING_TREZOR
        ,
        crypto::Trezor& bitcoin
#endif
    );
    static api::Identity* Identity(const api::client::Wallet& wallet);
    static api::client::Issuer* Issuer(
        const api::client::Wallet& wallet,
        const Identifier& nymID,
        const proto::Issuer& serialized);
    static api::client::Issuer* Issuer(
        const api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& issuerID);
    static ui::IssuerItem* IssuerItem(
        const ui::implementation::AccountSummaryParent& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Wallet& wallet,
        const api::storage::Storage& storage,
        const proto::ContactItemType currency,
        const Identifier& id);
    static ui::ActivityThreadItem* MailItem(
        const ui::implementation::ActivityThreadParent& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ActivityThreadRowID& id,
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
        const ui::implementation::ActivityThreadRowID& id,
        const Identifier& nymID,
        const api::Activity& activity,
        const std::chrono::system_clock::time_point& time);
    static ui::MessagableList* MessagableList(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const Identifier& nymID);
    static api::NativeInternal* Native(
        Flag& running,
        const ArgList& args,
        const bool recover,
        const bool serverMode,
        const std::chrono::seconds gcInterval,
        OTCaller* externalPasswordCallback = nullptr);
    static OTCallback* NullCallback();
    static crypto::OpenSSL* OpenSSL();
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
        const ui::implementation::ActivityThreadRowID& id,
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
    static crypto::Secp256k1* Secp256k1(
        const api::crypto::Util& util,
        const crypto::Trezor& ecdsa);
    static api::client::ServerAction* ServerAction(
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const ContextLockCallback& lockCallback);
    static api::Settings* Settings();
    static api::Settings* Settings(const String& path);
    static crypto::Sodium* Sodium();
    static api::storage::StorageInternal* Storage(
        const Flag& running,
        const StorageConfig& config,
        const String& primary,
        const bool migrate,
        const String& previous,
        const Digest& hash,
        const Random& random);
    static api::crypto::Symmetric* Symmetric(crypto::SymmetricProvider& sodium);
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
    static crypto::Trezor* Trezor(const api::Native& native);
    static api::UI* UI(
        const api::client::Sync& sync,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const api::network::ZMQ& connection,
        const api::storage::Storage& storage,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const network::zeromq::Context& zmq,
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
