// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs
{
class Factory
{
public:
    static opentxs::Armored* Armored();
    static opentxs::Armored* Armored(const opentxs::Data& input);
    static opentxs::Armored* Armored(const opentxs::String& input);
    static opentxs::Armored* Armored(const opentxs::OTEnvelope& input);
    static identity::internal::Authority* Authority(
        const api::Core& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const proto::KeyMode mode,
        const proto::Authority& serialized,
        const opentxs::PasswordPrompt& reason);
    static identity::internal::Authority* Authority(
        const api::Core& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const NymParameters& nymParameters,
        const VersionNumber nymVersion,
        const opentxs::PasswordPrompt& reason);
    static ui::implementation::AccountListRowInternal* AccountListItem(
        const opentxs::PasswordPrompt& reason,
        const ui::implementation::AccountListInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::AccountListRowID& rowID,
        const ui::implementation::AccountListSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static ui::implementation::AccountSummaryExternalInterface* AccountSummary(
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const proto::ContactItemType currency
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback = {},
        const RowCallbacks removeCallback = {}
#endif
    );
    static ui::implementation::IssuerItemRowInternal* AccountSummaryItem(
        const opentxs::PasswordPrompt& reason,
        const ui::implementation::IssuerItemInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::IssuerItemRowID& rowID,
        const ui::implementation::IssuerItemSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static api::client::internal::Activity* Activity(
        const api::Core& api,
        const api::client::Contacts& contact);
    static ui::implementation::ActivitySummaryRowInternal* ActivitySummaryItem(
        const ui::implementation::ActivitySummaryInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ui::implementation::ActivitySummaryRowID& rowID,
        const ui::implementation::ActivitySummarySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const Flag& running);
    static api::crypto::internal::Asymmetric* AsymmetricAPI(
        const api::internal::Core& api);
    static ui::implementation::AccountActivityRowInternal* BalanceItem(
        const ui::implementation::AccountActivityInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::AccountActivityRowID& rowID,
        const ui::implementation::AccountActivitySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const identifier::Nym& nymID,
        const Identifier& accountID);
    static crypto::Bitcoin* Bitcoin(const api::Crypto& crypto);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    static api::client::Blockchain* BlockchainAPI(
        const api::Core& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contacts);
    static api::client::blockchain::internal::BalanceList*
    BlockchainBalanceList(
        const api::client::internal::Blockchain& parent,
        const blockchain::Type chain);
    static api::client::blockchain::internal::BalanceTree*
    BlockchainBalanceTree(
        const api::client::blockchain::internal::BalanceList& parent,
        const identifier::Nym& id,
        const std::set<OTIdentifier>& hdAccounts,
        const std::set<OTIdentifier>& importedAccounts,
        const std::set<OTIdentifier>& paymentCodeAccounts);
    static api::client::blockchain::internal::HD* BlockchainHDBalanceNode(
        const api::client::blockchain::internal::BalanceTree& parent,
        const proto::HDPath& path,
        Identifier& id);
    static api::client::blockchain::internal::HD* BlockchainHDBalanceNode(
        const api::client::blockchain::internal::BalanceTree& parent,
        const proto::HDAccount& serialized,
        Identifier& id);
#endif
    static internal::ClientContext* ClientContext(
        const api::Core& api,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Server& server);
    static internal::ClientContext* ClientContext(
        const api::Core& api,
        const proto::Context& serialized,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Server& server);
    static api::client::internal::Manager* ClientManager(
        const api::internal::Context& parent,
        Flag& running,
        const ArgList& args,
        const api::Settings& config,
        const api::Crypto& crypto,
        const network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);
    static identity::credential::internal::Contact* ContactCredential(
        const api::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
    static identity::credential::internal::Contact* ContactCredential(
        const api::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential,
        const opentxs::PasswordPrompt& reason);
    static ui::implementation::ContactListRowInternal* ContactListItem(
        const ui::implementation::ContactListInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ContactListRowID& rowID,
        const ui::implementation::ContactListSortKey& key);
    static api::client::internal::Contacts* ContactAPI(
        const api::client::Manager& api);
    static ui::implementation::ContactSubsectionRowInternal* ContactItemWidget(
        const ui::implementation::ContactSubsectionInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ContactSubsectionRowID& rowID,
        const ui::implementation::ContactSubsectionSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static ui::implementation::ContactRowInternal* ContactSectionWidget(
        const ui::implementation::ContactInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ContactRowID& rowID,
        const ui::implementation::ContactSortKey& key,
        const ui::implementation::CustomData& custom
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback = {},
        const RowCallbacks removeCallback = {}
#endif
    );
    static ui::implementation::ContactSectionRowInternal*
    ContactSubsectionWidget(
        const ui::implementation::ContactSectionInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ContactSectionRowID& rowID,
        const ui::implementation::ContactSectionSortKey& key,
        const ui::implementation::CustomData& custom
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback = {},
        const RowCallbacks removeCallback = {}
#endif
    );
    static api::internal::Context* Context(
        Flag& running,
        const ArgList& args,
        const std::chrono::seconds gcInterval,
        OTCaller* externalPasswordCallback = nullptr);
    template <class C>
    static C* Credential(
        const api::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const VersionNumber version,
        const NymParameters& parameters,
        const proto::CredentialRole role,
        const opentxs::PasswordPrompt& reason);
    template <class C>
    static C* Credential(
        const api::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized,
        const proto::KeyMode mode,
        const proto::CredentialRole role,
        const opentxs::PasswordPrompt& reason);
    static api::Crypto* Crypto(const api::Settings& settings);
    static api::crypto::Config* CryptoConfig(const api::Settings& settings);
    static network::zeromq::socket::Dealer* DealerSocket(
        const network::zeromq::Context& context,
        const bool direction,
        const network::zeromq::ListenCallback& callback);
    static api::network::Dht* Dht(
        const bool defaultEnable,
        const api::Core& api,
        std::int64_t& nymPublishInterval,
        std::int64_t& nymRefreshInterval,
        std::int64_t& serverPublishInterval,
        std::int64_t& serverRefreshInterval,
        std::int64_t& unitPublishInterval,
        std::int64_t& unitRefreshInterval);
    static crypto::key::Ed25519* Ed25519Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey,
        const opentxs::PasswordPrompt& reason);
    static crypto::key::Ed25519* Ed25519Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::KeyRole role,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    static crypto::key::Ed25519* Ed25519Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const OTPassword& privateKey,
        const OTPassword& chainCode,
        const Data& publicKey,
        const proto::HDPath& path,
        const Bip32Fingerprint parent,
        const proto::KeyRole role,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    static api::crypto::Encode* Encode(const api::Crypto& crypto);
    static api::Endpoints* Endpoints(
        const network::zeromq::Context& zmq,
        const int instance);
    static api::internal::Factory* FactoryAPI(const api::internal::Core& api);
    static api::internal::Factory* FactoryAPIClient(
        const api::client::internal::Manager& api);
    static api::crypto::Hash* Hash(
        const api::crypto::Encode& encode,
        const crypto::HashingProvider& ssl,
        const crypto::HashingProvider& sodium
#if OT_CRYPTO_USING_TREZOR
        ,
        const crypto::Ripemd160& bitcoin
#endif
    );
#if OT_CRYPTO_WITH_BIP39
    static api::HDSeed* HDSeed(
        const api::Factory& factory,
        const api::crypto::Asymmetric& asymmetric,
        const api::crypto::Symmetric& symmetric,
        const api::storage::Storage& storage,
        const crypto::Bip32& bip32,
        const crypto::Bip39& bip39);
#endif
    static api::client::Issuer* Issuer(
        const api::Wallet& wallet,
        const identifier::Nym& nymID,
        const proto::Issuer& serialized);
    static api::client::Issuer* Issuer(
        const api::Wallet& wallet,
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID);
    static ui::implementation::AccountSummaryRowInternal* IssuerItem(
        const ui::implementation::AccountSummaryInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::AccountSummaryRowID& rowID,
        const ui::implementation::AccountSummarySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const proto::ContactItemType currency
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback = {},
        const RowCallbacks removeCallback = {}
#endif
    );
    static crypto::key::Keypair* Keypair();
    static crypto::key::Keypair* Keypair(
        const api::Core& api,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const proto::KeyRole role,
        const opentxs::PasswordPrompt& reason);
    static crypto::key::Keypair* Keypair(
        const api::Core& api,
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey,
        const opentxs::PasswordPrompt& reason);
    static crypto::key::Keypair* Keypair(
        const api::Core& api,
        const proto::AsymmetricKey& serializedPubkey,
        const opentxs::PasswordPrompt& reason);
    static api::Legacy* Legacy();
    static api::internal::Log* Log(
        const network::zeromq::Context& zmq,
        const std::string& endpoint);
    static ui::implementation::ActivityThreadRowInternal* MailItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const bool loading,
        const bool pending);
    static ui::implementation::ActivityThreadRowInternal* MailItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static opentxs::ManagedNumber* ManagedNumber(
        const TransactionNumber number,
        opentxs::ServerContext& context);
    static ui::implementation::MessagableExternalInterface* MessagableList(
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback = {},
        const RowCallbacks removeCallback = {}
#endif
    );
#if OT_CASH
    static blind::Mint* MintLucre(const api::Core& core);
    static blind::Mint* MintLucre(
        const api::Core& core,
        const String& strNotaryID,
        const String& strInstrumentDefinitionID);
    static blind::Mint* MintLucre(
        const api::Core& core,
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID);
#endif
    static OTCallback* NullCallback();
    static identity::internal::Nym* Nym(
        const api::Core& api,
        const NymParameters& nymParameters,
        const proto::ContactItemType type,
        const std::string name,
        const opentxs::PasswordPrompt& reason);
    static identity::internal::Nym* Nym(
        const api::Core& api,
        const proto::Nym& serialized,
        const std::string& alias,
        const opentxs::PasswordPrompt& reason);
    static internal::NymFile* NymFile(
        const api::Core& core,
        Nym_p targetNym,
        Nym_p signerNym);
    static identity::Source* NymIDSource(
        const api::Core& api,
        NymParameters& parameters,
        const opentxs::PasswordPrompt& reason);
    static identity::Source* NymIDSource(
        const api::Core& api,
        const proto::NymIDSource& serialized,
        const opentxs::PasswordPrompt& reason);
    static crypto::OpenSSL* OpenSSL(const api::Crypto& crypto);
    static otx::client::internal::Operation* Operation(
        const api::client::Manager& api,
        const identifier::Nym& nym,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason);
    static api::client::OTX* OTX(
        const Flag& running,
        const api::client::Manager& api,
        OTClient& otclient,
        const ContextLockCallback& lockCallback);
    static opentxs::PasswordPrompt* PasswordPrompt(
        const api::internal::Core& api,
        const std::string& text);
    static api::client::internal::Pair* PairAPI(
        const Flag& running,
        const api::client::Manager& client);
    static network::zeromq::socket::Pair* PairSocket(
        const network::zeromq::Context& context,
        const network::zeromq::ListenCallback& callback,
        const bool startThread);
    static network::zeromq::socket::Pair* PairSocket(
        const network::zeromq::ListenCallback& callback,
        const network::zeromq::socket::Pair& peer,
        const bool startThread);
    static network::zeromq::socket::Pair* PairSocket(
        const network::zeromq::Context& context,
        const network::zeromq::ListenCallback& callback,
        const std::string& endpoint);
    static ui::implementation::PayableExternalInterface* PayableList(
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const proto::ContactItemType& currency
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback = {},
        const RowCallbacks removeCallback = {}
#endif
    );
    static ui::implementation::PayableListRowInternal* PayableListItem(
        const ui::implementation::PayableInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::PayableListRowID& rowID,
        const ui::implementation::PayableListSortKey& key,
        const std::string& paymentcode,
        const proto::ContactItemType& currency);
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    static opentxs::PaymentCode* PaymentCode(
        const api::Core& api,
        const std::string& base58,
        const opentxs::PasswordPrompt& reason);
    static opentxs::PaymentCode* PaymentCode(
        const api::Core& api,
        const proto::PaymentCode& serialized,
        const opentxs::PasswordPrompt& reason);
    static opentxs::PaymentCode* PaymentCode(
        const api::Core& api,
        const std::string& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0);
#endif
    static ui::implementation::ActivityThreadRowInternal* PaymentItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static opentxs::PeerObject* PeerObject(
        const api::Core& api,
        const Nym_p& senderNym,
        const std::string& message,
        const opentxs::PasswordPrompt& reason);
    static opentxs::PeerObject* PeerObject(
        const api::Core& api,
        const Nym_p& senderNym,
        const std::string& payment,
        const bool isPayment,
        const opentxs::PasswordPrompt& reason);
#if OT_CASH
    static opentxs::PeerObject* PeerObject(
        const api::Core& api,
        const Nym_p& senderNym,
        const std::shared_ptr<blind::Purse> purse,
        const opentxs::PasswordPrompt& reason);
#endif
    static opentxs::PeerObject* PeerObject(
        const api::Core& api,
        const std::shared_ptr<const PeerRequest> request,
        const std::shared_ptr<const PeerReply> reply,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
    static opentxs::PeerObject* PeerObject(
        const api::Core& api,
        const std::shared_ptr<const PeerRequest> request,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
    static opentxs::PeerObject* PeerObject(
        const api::client::Contacts& contacts,
        const api::Core& api,
        const Nym_p& signerNym,
        const proto::PeerObject& serialized,
        const opentxs::PasswordPrompt& reason);
    static opentxs::PeerObject* PeerObject(
        const api::client::Contacts& contacts,
        const api::Core& api,
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason);
    static ui::implementation::ActivityThreadRowInternal* PendingSend(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static opentxs::PIDFile* PIDFile(const std::string& path);
    static opentxs::network::zeromq::Pipeline* Pipeline(
        const api::Core& api,
        const network::zeromq::Context& context,
        std::function<void(network::zeromq::Message&)> callback);
    static identity::credential::internal::Primary* PrimaryCredential(
        const api::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
    static identity::credential::internal::Primary* PrimaryCredential(
        const api::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const proto::Credential& credential,
        const opentxs::PasswordPrompt& reason);
    static ui::implementation::ProfileSubsectionRowInternal* ProfileItemWidget(
        const ui::implementation::ProfileSubsectionInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ProfileSubsectionRowID& rowID,
        const ui::implementation::ProfileSubsectionSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static ui::implementation::ProfileRowInternal* ProfileSectionWidget(
        const ui::implementation::ProfileInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ProfileRowID& rowID,
        const ui::implementation::ProfileSortKey& key,
        const ui::implementation::CustomData& custom
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback = {},
        const RowCallbacks removeCallback = {}
#endif
    );
    static ui::implementation::ProfileSectionRowInternal*
    ProfileSubsectionWidget(
        const ui::implementation::ProfileSectionInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ui::implementation::ProfileSectionRowID& rowID,
        const ui::implementation::ProfileSectionSortKey& key,
        const ui::implementation::CustomData& custom
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback = {},
        const RowCallbacks removeCallback = {}
#endif
    );
#if OT_CASH
    static blind::Purse* Purse(
        const api::Core& api,
        const proto::Purse& serialized);
    static blind::Purse* Purse(
        const api::Core& api,
        const opentxs::ServerContext& context,
        const proto::CashType type,
        const blind::Mint& mint,
        const Amount totalValue,
        const opentxs::PasswordPrompt& reason);
    static blind::Purse* Purse(
        const api::Core& api,
        const identity::Nym& owner,
        const identifier::Server& server,
        const identity::Nym& serverNym,
        const proto::CashType type,
        const blind::Mint& mint,
        const Amount totalValue,
        const opentxs::PasswordPrompt& reason);
    static blind::Purse* Purse(
        const api::Core& api,
        const blind::Purse& request,
        const identity::Nym& requester,
        const opentxs::PasswordPrompt& reason);
    static blind::Purse* Purse(
        const api::Core& api,
        const identity::Nym& owner,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const proto::CashType type,
        const opentxs::PasswordPrompt& reason);
#endif
    static network::zeromq::socket::Publish* PublishSocket(
        const network::zeromq::Context& context);
    static network::zeromq::socket::Pull* PullSocket(
        const network::zeromq::Context& context,
        const bool direction);
    static network::zeromq::socket::Pull* PullSocket(
        const network::zeromq::Context& context,
        const bool direction,
        const network::zeromq::ListenCallback& callback);
    static network::zeromq::socket::Push* PushSocket(
        const network::zeromq::Context& context,
        const bool direction);
    static network::zeromq::socket::Reply* ReplySocket(
        const network::zeromq::Context& context,
        const bool direction,
        const network::zeromq::ReplyCallback& callback);
    static network::zeromq::socket::Request* RequestSocket(
        const network::zeromq::Context& context);
    static rpc::internal::RPC* RPC(const api::Context& native);
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    static crypto::key::RSA* RSAKey(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKey& serializedKey);
    static crypto::key::RSA* RSAKey(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::KeyRole role);
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
    static network::zeromq::socket::Router* RouterSocket(
        const network::zeromq::Context& context,
        const bool direction,
        const network::zeromq::ListenCallback& callback);
    static identity::credential::internal::Secondary* SecondaryCredential(
        const api::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
    static identity::credential::internal::Secondary* SecondaryCredential(
        const api::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential,
        const opentxs::PasswordPrompt& reason);
    static crypto::Secp256k1* Secp256k1(
        const api::Crypto& crypto,
        const api::crypto::Util& util,
        const crypto::EcdsaProvider& ecdsa);
    static crypto::key::Secp256k1* Secp256k1Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey,
        const opentxs::PasswordPrompt& reason);
    static crypto::key::Secp256k1* Secp256k1Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::KeyRole role,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    static crypto::key::Secp256k1* Secp256k1Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const OTPassword& privateKey,
        const OTPassword& chainCode,
        const Data& publicKey,
        const proto::HDPath& path,
        const Bip32Fingerprint parent,
        const proto::KeyRole role,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    static api::client::ServerAction* ServerAction(
        const api::client::Manager& api,
        const ContextLockCallback& lockCallback);
    static internal::ServerContext* ServerContext(
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& requestSent,
        const network::zeromq::socket::Publish& replyReceived,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Server& server,
        network::ServerConnection& connection);
    static internal::ServerContext* ServerContext(
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& requestSent,
        const network::zeromq::socket::Publish& replyReceived,
        const proto::Context& serialized,
        const Nym_p& local,
        const Nym_p& remote,
        network::ServerConnection& connection);
    static api::server::Manager* ServerManager(
        const api::internal::Context& parent,
        Flag& running,
        const ArgList& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);
    static api::Settings* Settings();
    static api::Settings* Settings(const String& path);
    static crypto::Sodium* Sodium(const api::Crypto& crypto);
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
#if OT_STORAGE_LMDB
    static opentxs::api::storage::Plugin* StorageLMDB(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
#endif
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
    static network::zeromq::socket::Subscribe* SubscribeSocket(
        const network::zeromq::Context& context,
        const network::zeromq::ListenCallback& callback);
    static api::crypto::Symmetric* Symmetric(const api::Core& api);
    static crypto::key::Symmetric* SymmetricKey();
    static crypto::key::Symmetric* SymmetricKey(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& reason,
        const proto::SymmetricMode mode);
    static crypto::key::Symmetric* SymmetricKey(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized);
    static crypto::key::Symmetric* SymmetricKey(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const OTPassword& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const proto::SymmetricKeyType type);
    static crypto::key::Symmetric* SymmetricKey(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const OTPassword& raw,
        const opentxs::PasswordPrompt& reason);
#if OT_CASH
    static blind::Token* Token(
        const api::Core& api,
        blind::Purse& purse,
        const proto::Token& serialized);
    static blind::Token* Token(
        const api::Core& api,
        const identity::Nym& owner,
        const blind::Mint& mint,
        const std::uint64_t value,
        blind::Purse& purse,
        const OTPassword& primaryPassword,
        const OTPassword& secondaryPassword);
#endif
    static crypto::Trezor* Trezor(const api::Crypto& crypto);
    static api::client::UI* UI(
        const api::client::Manager& api,
        const Flag& running
#if OT_QT
        ,
        const bool qt
#endif
    );
    static identity::credential::internal::Verification* VerificationCredential(
        const api::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason);
    static identity::credential::internal::Verification* VerificationCredential(
        const api::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential,
        const opentxs::PasswordPrompt& reason);
    static identity::wot::verification::internal::Group* VerificationGroup(
        identity::wot::verification::internal::Set& parent,
        const VersionNumber version,
        bool external);
    static identity::wot::verification::internal::Group* VerificationGroup(
        identity::wot::verification::internal::Set& parent,
        const proto::VerificationGroup& serialized,
        bool external);
    static identity::wot::verification::internal::Item* VerificationItem(
        const identity::wot::verification::internal::Nym& parent,
        const Identifier& claim,
        const identity::Nym& signer,
        const opentxs::PasswordPrompt& reason,
        const bool value,
        const Time start,
        const Time end,
        const VersionNumber version);
    static identity::wot::verification::internal::Item* VerificationItem(
        const identity::wot::verification::internal::Nym& parent,
        const proto::Verification& serialized);
    static identity::wot::verification::internal::Nym* VerificationNym(
        identity::wot::verification::internal::Group& parent,
        const identifier::Nym& nym,
        const VersionNumber version);
    static identity::wot::verification::internal::Nym* VerificationNym(
        identity::wot::verification::internal::Group& parent,
        const proto::VerificationIdentity& serialized);
    static identity::wot::verification::internal::Set* VerificationSet(
        const api::Core& api,
        const identifier::Nym& nym,
        const VersionNumber version);
    static identity::wot::verification::internal::Set* VerificationSet(
        const api::Core& api,
        const identifier::Nym& nym,
        const proto::VerificationSet& serialized);
    static api::Wallet* Wallet(const api::client::Manager& client);
    static api::Wallet* Wallet(const api::server::Manager& server);
    static api::client::Workflow* Workflow(
        const api::Core& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contact);
    static api::network::ZAP* ZAP(const network::zeromq::Context& context);
    static api::network::ZMQ* ZMQ(const api::Core& api, const Flag& running);
    static network::zeromq::Context* ZMQContext();
    static network::zeromq::Frame* ZMQFrame();
    static network::zeromq::Frame* ZMQFrame(
        const void* data,
        const std::size_t size);
    static network::zeromq::Frame* ZMQFrame(const ProtobufType& data);
    static network::zeromq::Message* ZMQMessage();
    static network::zeromq::Message* ZMQMessage(
        const void* data,
        const std::size_t size);
    static network::zeromq::Message* ZMQMessage(const ProtobufType& data);
};
}  // namespace opentxs
