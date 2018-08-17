// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs
{
class Factory
{
public:
    static ui::implementation::AccountActivityExternalInterface*
    AccountActivity(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Sync& sync,
        const api::client::Workflow& workflow,
        const api::client::Contacts& contact,
        const api::storage::Storage& storage,
        const api::Core& core,
        const Identifier& nymID,
        const Identifier& accountID);
    static ui::implementation::AccountSummaryExternalInterface* AccountSummary(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::network::ZMQ& connection,
        const api::storage::Storage& storage,
        const api::client::Contacts& contact,
        const api::Core& core,
        const Identifier& nymID,
        const proto::ContactItemType currency);
    static ui::implementation::IssuerItemRowInternal* AccountSummaryItem(
        const ui::implementation::IssuerItemInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ui::implementation::IssuerItemRowID& rowID,
        const ui::implementation::IssuerItemSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::Wallet& wallet,
        const api::storage::Storage& storage);
    static api::client::internal::Activity* Activity(
        const api::storage::Storage& storage,
        const api::client::Contacts& contact,
        const api::Core& core,
        const network::zeromq::Context& zmq);
    static ui::implementation::ActivitySummaryExternalInterface*
    ActivitySummary(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Activity& activity,
        const api::client::Contacts& contact,
        const Flag& running,
        const Identifier& nymID);
    static ui::implementation::ActivitySummaryRowInternal* ActivitySummaryItem(
        const ui::implementation::ActivitySummaryInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Activity& activity,
        const api::client::Contacts& contact,
        const Identifier& nymID,
        const ui::implementation::ActivitySummaryRowID& rowID,
        const ui::implementation::ActivitySummarySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const Flag& running);
    static ui::ActivityThread* ActivityThread(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Sync& sync,
        const api::client::Activity& activity,
        const api::client::Contacts& contact,
        const Identifier& nymID,
        const Identifier& threadID);
    static ui::implementation::AccountActivityRowInternal* BalanceItem(
        const ui::implementation::AccountActivityInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ui::implementation::AccountActivityRowID& rowID,
        const ui::implementation::AccountActivitySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::client::Sync& sync,
        const api::Core& core,
        const Identifier& nymID,
        const Identifier& accountID);
    static crypto::Bitcoin* Bitcoin(const api::Crypto& crypto);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    static api::client::Blockchain* Blockchain(
        const api::client::Activity& activity,
        const api::Crypto& crypto,
        const api::HDSeed& seeds,
        const api::storage::Storage& storage,
        const api::Wallet& wallet);
#endif
    static api::client::Cash* Cash(const api::Core& core);
    static api::client::internal::Manager* ClientManager(
        const Flag& running,
        const ArgList& args,
        const api::Settings& config,
        const api::Crypto& crypto,
        const api::Legacy& legacy,
        const network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);
    static ui::implementation::ContactListExternalInterface* ContactList(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const Identifier& nymID);
    static ui::implementation::ContactListRowInternal* ContactListItem(
        const ui::implementation::ContactListInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ui::implementation::ContactListRowID& rowID,
        const ui::implementation::ContactListSortKey& key);
    static api::client::internal::Contacts* Contacts(
        const api::storage::Storage& storage,
        const api::Factory& factory,
        const api::Wallet& wallet,
        const network::zeromq::Context& context);
    static ui::implementation::ContactExternalInterface* ContactWidget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const Identifier& contactID);
    static ui::implementation::ContactSubsectionRowInternal* ContactItemWidget(
        const ui::implementation::ContactSubsectionInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ui::implementation::ContactSubsectionRowID& rowID,
        const ui::implementation::ContactSubsectionSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static ui::implementation::ContactRowInternal* ContactSectionWidget(
        const ui::implementation::ContactInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ui::implementation::ContactRowID& rowID,
        const ui::implementation::ContactSortKey& key,
        const ui::implementation::CustomData& custom);
    static ui::implementation::ContactSectionRowInternal*
    ContactSubsectionWidget(
        const ui::implementation::ContactSectionInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ui::implementation::ContactSectionRowID& rowID,
        const ui::implementation::ContactSectionSortKey& key,
        const ui::implementation::CustomData& custom);
    static api::Crypto* Crypto(const api::Settings& settings);
    static api::crypto::Config* CryptoConfig(const api::Settings& settings);
    static api::network::Dht* Dht(
        const int instance,
        const bool defaultEnable,
        const api::Settings& settings,
        const api::Wallet& wallet,
        const network::zeromq::Context& zmq,
        std::int64_t& nymPublishInterval,
        std::int64_t& nymRefreshInterval,
        std::int64_t& serverPublishInterval,
        std::int64_t& serverRefreshInterval,
        std::int64_t& unitPublishInterval,
        std::int64_t& unitRefreshInterval);
    static crypto::key::Ed25519* Ed25519Key(
        const proto::AsymmetricKey& serializedKey);
    static crypto::key::Ed25519* Ed25519Key(const String& publicKey);
    static crypto::key::Ed25519* Ed25519Key(const proto::KeyRole role);
    static api::crypto::Encode* Encode(const crypto::EncodingProvider& base58);
    static api::Factory* FactoryAPI(
#if OT_CRYPTO_WITH_BIP39
        const api::HDSeed& seeds
#endif
    );
    static api::crypto::Hash* Hash(
        const api::crypto::Encode& encode,
        const crypto::HashingProvider& ssl,
        const crypto::HashingProvider& sodium
#if OT_CRYPTO_USING_TREZOR || OT_CRYPTO_USING_LIBBITCOIN
        ,
        const crypto::Ripemd160& bitcoin
#endif
    );
#if OT_CRYPTO_WITH_BIP39
    static api::HDSeed* HDSeed(
        const api::crypto::Symmetric& symmetric,
        const api::storage::Storage& storage,
        const crypto::Bip32& bip32,
        const crypto::Bip39& bip39,
        const crypto::LegacySymmetricProvider& aes);
#endif
    static api::Identity* Identity(const api::Wallet& wallet);
    static api::client::Issuer* Issuer(
        const api::Wallet& wallet,
        const Identifier& nymID,
        const proto::Issuer& serialized);
    static api::client::Issuer* Issuer(
        const api::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& issuerID);
    static ui::implementation::AccountSummaryRowInternal* IssuerItem(
        const ui::implementation::AccountSummaryInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ui::implementation::AccountSummaryRowID& rowID,
        const ui::implementation::AccountSummarySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::storage::Storage& storage,
        const api::Core& core,
        const proto::ContactItemType currency);
    static api::Legacy* Legacy();
    static ui::implementation::ActivityThreadRowInternal* MailItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const Identifier& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::client::Activity& activity,
        const bool loading,
        const bool pending);
    static ui::implementation::ActivityThreadRowInternal* MailItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const Identifier& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::client::Activity& activity);
    static ui::implementation::MessagableExternalInterface* MessagableList(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const api::client::Sync& sync,
        const Identifier& nymID);
    static api::internal::Native* Native(
        Flag& running,
        const ArgList& args,
        const bool recover,
        const bool serverMode,
        const std::chrono::seconds gcInterval,
        OTCaller* externalPasswordCallback = nullptr);
    static OTCallback* NullCallback();
    static internal::NymFile* NymFile(
        const api::Core& core,
        std::shared_ptr<const Nym> targetNym,
        std::shared_ptr<const Nym> signerNym);
    static crypto::OpenSSL* OpenSSL();
    static api::client::Pair* Pair(
        const Flag& running,
        const api::client::Sync& sync,
        const api::client::ServerAction& action,
        const api::Wallet& wallet,
        const api::Legacy& legacy,
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const network::zeromq::Context& context);
    static ui::implementation::PayableExternalInterface* PayableList(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const api::client::Sync& sync,
        const Identifier& nymID,
        const proto::ContactItemType& currency);
    static ui::implementation::PayableListRowInternal* PayableListItem(
        const ui::implementation::PayableInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ui::implementation::PayableListRowID& rowID,
        const ui::implementation::PayableListSortKey& key,
        const std::string& paymentcode,
        const proto::ContactItemType& currency);
    static ui::implementation::ActivityThreadRowInternal* PaymentItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const Identifier& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::client::Activity& activity);
    static ui::implementation::ProfileExternalInterface* ProfileWidget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const api::Wallet& wallet,
        const Identifier& nymID);
    static ui::implementation::ProfileSubsectionRowInternal* ProfileItemWidget(
        const ui::implementation::ProfileSubsectionInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ui::implementation::ProfileSubsectionRowID& rowID,
        const ui::implementation::ProfileSubsectionSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::Wallet& wallet);
    static ui::implementation::ProfileRowInternal* ProfileSectionWidget(
        const ui::implementation::ProfileInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ui::implementation::ProfileRowID& rowID,
        const ui::implementation::ProfileSortKey& key,
        const ui::implementation::CustomData& custom,
        const api::Wallet& wallet);
    static ui::implementation::ProfileSectionRowInternal*
    ProfileSubsectionWidget(
        const ui::implementation::ProfileSectionInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ui::implementation::ProfileSectionRowID& rowID,
        const ui::implementation::ProfileSectionSortKey& key,
        const ui::implementation::CustomData& custom,
        const api::Wallet& wallet);
    static crypto::key::RSA* RSAKey(const proto::AsymmetricKey& serializedKey);
    static crypto::key::RSA* RSAKey(const String& publicKey);
    static crypto::key::RSA* RSAKey(const proto::KeyRole role);
    static crypto::Secp256k1* Secp256k1(
        const api::crypto::Util& util,
        const crypto::EcdsaProvider& ecdsa);
    static crypto::key::Secp256k1* Secp256k1Key(
        const proto::AsymmetricKey& serializedKey);
    static crypto::key::Secp256k1* Secp256k1Key(const String& publicKey);
    static crypto::key::Secp256k1* Secp256k1Key(const proto::KeyRole role);
    static api::client::ServerAction* ServerAction(
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const api::client::Workflow& workflow,
        const api::Core& core,
        const ContextLockCallback& lockCallback);
    static api::server::Manager* ServerManager(
        const Flag& running,
        const ArgList& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);
    static api::Settings* Settings();
    static api::Settings* Settings(const String& path);
    static crypto::Sodium* Sodium();
    static api::storage::StorageInternal* Storage(
        const Flag& running,
        const api::Crypto& crypto,
        const api::Settings& config,
        const std::string& dataFolder,
        const String& defaultPluginCLI,
        const String& archiveDirectoryCLI,
        const std::chrono::seconds gcIntervalCLI,
        String& encryptedDirectoryCLI,
        StorageConfig& storageConfig);
    static api::crypto::Symmetric* Symmetric(crypto::SymmetricProvider& sodium);
#if OT_STORAGE_FS
    static opentxs::api::storage::Plugin* StorageFSArchive(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket,
        const std::string& folder,
        crypto::key::Symmetric& key);
    static opentxs::api::storage::Plugin* StorageFSGC(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
#endif
    static opentxs::api::storage::Plugin* StorageMemDB(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
    static opentxs::api::storage::Multiplex* StorageMultiplex(
        const api::storage::Storage& storage,
        const Flag& primaryBucket,
        const StorageConfig& config,
        const String& primary,
        const bool migrate,
        const String& previous,
        const Digest& hash,
        const Random& random);
#if OT_STORAGE_SQLITE
    static opentxs::api::storage::Plugin* StorageSqlite3(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
#endif
    static api::client::Sync* Sync(
        const Flag& running,
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const api::client::Contacts& contacts,
        const api::Settings& config,
        const api::client::Manager& api,
        const api::Legacy& legacy,
        const api::Wallet& wallet,
        const api::client::Workflow& workflow,
        const api::crypto::Encode& encoding,
        const api::storage::Storage& storage,
        const network::zeromq::Context& zmq,
        const ContextLockCallback& lockCallback);
    static crypto::Trezor* Trezor(const api::Crypto& crypto);
    static api::client::UI* UI(
        const api::client::Sync& sync,
        const api::Wallet& wallet,
        const api::client::Workflow& workflow,
        const api::network::ZMQ& connection,
        const api::storage::Storage& storage,
        const api::client::Activity& activity,
        const api::client::Contacts& contact,
        const api::Core& core,
        const network::zeromq::Context& zmq,
        const Flag& running);
    static api::Wallet* Wallet(const api::client::Manager& client);
    static api::Wallet* Wallet(const api::server::Manager& server);
    static api::client::Workflow* Workflow(
        const api::client::Activity& activity,
        const api::client::Contacts& contact,
        const api::Core& core,
        const api::storage::Storage& storage,
        const network::zeromq::Context& zmq);
    static api::network::ZMQ* ZMQ(const api::Core& api, const Flag& running);
};
}  // namespace opentxs
