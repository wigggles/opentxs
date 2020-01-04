// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/crypto/Crypto.hpp"

namespace opentxs::api::implementation
{
class Factory : virtual public api::internal::Factory
{
public:
    OTArmored Armored() const final;
    OTArmored Armored(const std::string& input) const final;
    OTArmored Armored(const opentxs::Data& input) const final;
    OTArmored Armored(const opentxs::String& input) const final;
    OTArmored Armored(const opentxs::crypto::Envelope& input) const final;
    OTArmored Armored(const ProtobufType& input) const final;
    OTString Armored(const ProtobufType& input, const std::string& header)
        const final;
    const api::crypto::internal::Asymmetric& Asymmetric() const final
    {
        return asymmetric_;
    }
    OTAsymmetricKey AsymmetricKey(
        const NymParameters& params,
        const opentxs::PasswordPrompt& reason,
        const proto::KeyRole role,
        const VersionNumber version) const final;
    OTAsymmetricKey AsymmetricKey(
        const proto::AsymmetricKey& serialized) const final;
    OTBailmentNotice BailmentNotice(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const opentxs::Identifier& requestID,
        const std::string& txid,
        const Amount& amount,
        const opentxs::PasswordPrompt& reason) const noexcept(false) final;
    OTBailmentNotice BailmentNotice(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false) final;
    OTBailmentReply BailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false) final;
    OTBailmentReply BailmentReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false) final;
    OTBailmentRequest BailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const identifier::UnitDefinition& unit,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false) final;
    OTBailmentRequest BailmentRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false) final;
    std::unique_ptr<opentxs::Basket> Basket() const final;
    std::unique_ptr<opentxs::Basket> Basket(
        std::int32_t nCount,
        std::int64_t lMinimumTransferAmount) const final;
    OTBasketContract BasketContract(
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::uint64_t weight,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version) const noexcept(false) final;
    OTBasketContract BasketContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false) final;
    std::unique_ptr<OTPassword> BinarySecret() const final;
#if OT_BLOCKCHAIN
    OTBlockchainAddress BlockchainAddress(
        const blockchain::p2p::Protocol protocol,
        const blockchain::p2p::Network network,
        const opentxs::Data& bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const std::set<blockchain::p2p::Service>& services) const final;
    OTBlockchainAddress BlockchainAddress(
        const blockchain::p2p::Address::SerializedType& serialized) const final;
    std::unique_ptr<blockchain::block::Header> BlockHeader(
        const proto::BlockchainBlockHeader& serialized) const final;
    std::unique_ptr<blockchain::block::Header> BlockHeader(
        const blockchain::Type type,
        const opentxs::Data& raw) const final;
    std::unique_ptr<blockchain::block::Header> BlockHeader(
        const blockchain::Type type,
        const blockchain::block::Hash& hash,
        const blockchain::block::Hash& parent,
        const blockchain::block::Height height) const final;
#endif  // OT_BLOCKCHAIN
    std::unique_ptr<opentxs::Cheque> Cheque(
        const OTTransaction& receipt) const final;
    std::unique_ptr<opentxs::Cheque> Cheque() const final;
    std::unique_ptr<opentxs::Cheque> Cheque(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const final;
    OTConnectionReply ConnectionReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key,
        const opentxs::PasswordPrompt& reason) const noexcept(false) final;
    OTConnectionReply ConnectionReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false) final;
    OTConnectionRequest ConnectionRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const proto::ConnectionInfoType type,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false) final;
    OTConnectionRequest ConnectionRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false) final;
    std::unique_ptr<opentxs::Contract> Contract(
        const String& strCronItem) const final;
    std::unique_ptr<OTCron> Cron() const override;
    std::unique_ptr<OTCronItem> CronItem(const String& strCronItem) const final;
    OTCurrencyContract CurrencyContract(
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::string& tla,
        const std::uint32_t power,
        const std::string& fraction,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) const noexcept(false) final;
    OTCurrencyContract CurrencyContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false) final;
    OTData Data() const final;
    OTData Data(const opentxs::Armored& input) const final;
    OTData Data(const ProtobufType& input) const final;
    OTData Data(const opentxs::network::zeromq::Frame& input) const final;
    OTData Data(const std::uint8_t input) const final;
    OTData Data(const std::uint32_t input) const final;
    OTData Data(const std::string input, const StringStyle mode) const final;
    OTData Data(const std::vector<unsigned char>& input) const final;
    OTData Data(const std::vector<std::byte>& input) const final;
    OTData Data(ReadView input) const final;
    OTEnvelope Envelope() const noexcept final;
    OTEnvelope Envelope(const opentxs::Armored& ciphertext) const
        noexcept(false) final;
    OTEnvelope Envelope(
        const opentxs::crypto::Envelope::SerializedType& serialized) const
        noexcept(false) final;
    OTIdentifier Identifier() const final;
    OTIdentifier Identifier(const std::string& serialized) const final;
    OTIdentifier Identifier(const opentxs::String& serialized) const final;
    OTIdentifier Identifier(const opentxs::Contract& contract) const final;
    OTIdentifier Identifier(const opentxs::Item& item) const final;
    OTIdentifier Identifier(const ReadView bytes) const final;
    OTIdentifier Identifier(const ProtobufType& proto) const final;
    std::unique_ptr<opentxs::Item> Item(const String& serialized) const final;
    std::unique_ptr<opentxs::Item> Item(
        const std::string& serialized) const final;
    std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const opentxs::Item& theOwner) const final;
    std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner) const final;
    std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner,
        itemType theType,
        const opentxs::Identifier& pDestinationAcctID) const final;
    std::unique_ptr<opentxs::Item> Item(
        const String& strItem,
        const identifier::Server& theNotaryID,
        std::int64_t lTransactionNumber) const final;
    std::unique_ptr<opentxs::Item> Item(
        const OTTransaction& theOwner,
        itemType theType,
        const opentxs::Identifier& pDestinationAcctID) const final;
    OTKeypair Keypair(
        const NymParameters& nymParameters,
        const VersionNumber version,
        const proto::KeyRole role,
        const opentxs::PasswordPrompt& reason) const final;
    OTKeypair Keypair(
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey) const final;
    OTKeypair Keypair(const proto::AsymmetricKey& serializedPubkey) const final;
#if OT_CRYPTO_WITH_BIP32
    OTKeypair Keypair(
        const std::string& fingerprint,
        const Bip32Index nym,
        const Bip32Index credset,
        const Bip32Index credindex,
        const EcdsaCurve& curve,
        const proto::KeyRole role,
        const opentxs::PasswordPrompt& reason) const final;
#endif  // OT_CRYPTO_WITH_BIP32
    std::unique_ptr<opentxs::Ledger> Ledger(
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID) const final;
    std::unique_ptr<opentxs::Ledger> Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID) const final;
    std::unique_ptr<opentxs::Ledger> Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAcctID,
        const identifier::Server& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const final;
    std::unique_ptr<OTMarket> Market() const final;
    std::unique_ptr<OTMarket> Market(const char* szFilename) const final;
    virtual std::unique_ptr<OTMarket> Market(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_TYPE_ID,
        const std::int64_t& lScale) const final;
    std::unique_ptr<opentxs::Message> Message() const final;
#if OT_CASH
    std::unique_ptr<blind::Mint> Mint() const final;
    std::unique_ptr<blind::Mint> Mint(
        const String& strNotaryID,
        const String& strInstrumentDefinitionID) const final;
    std::unique_ptr<blind::Mint> Mint(
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID) const final;
#endif
    OTNymID NymID() const final;
    OTNymID NymID(const std::string& serialized) const final;
    OTNymID NymID(const opentxs::String& serialized) const final;
    OTNymID NymIDFromPaymentCode(const std::string& serialized) const final;
    std::unique_ptr<OTOffer> Offer() const final;
    std::unique_ptr<OTOffer> Offer(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const std::int64_t& MARKET_SCALE) const final;
    OTOutbailmentReply OutbailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false) final;
    OTOutbailmentReply OutbailmentReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false) final;
    OTOutbailmentRequest OutbailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const std::uint64_t& amount,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false) final;
    OTOutbailmentRequest OutbailmentRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false) final;
    OTPasswordPrompt PasswordPrompt(const std::string& text) const final;
    OTPasswordPrompt PasswordPrompt(
        const opentxs::PasswordPrompt& rhs) const final
    {
        return PasswordPrompt(rhs.GetDisplayString());
    }
    std::unique_ptr<OTPayment> Payment() const final;
    std::unique_ptr<OTPayment> Payment(const String& strPayment) const final;
    std::unique_ptr<OTPayment> Payment(
        const opentxs::Contract& contract,
        const opentxs::PasswordPrompt& reason) const final;
    OTPaymentCode PaymentCode(const std::string& base58) const noexcept final;
    OTPaymentCode PaymentCode(const proto::PaymentCode& serialized) const
        noexcept final;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
    OTPaymentCode PaymentCode(
        const std::string& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage,
        const std::uint8_t bitmessageVersion,
        const std::uint8_t bitmessageStream) const noexcept final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
    std::unique_ptr<OTPaymentPlan> PaymentPlan() const final;
    std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const final;
    std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const opentxs::Identifier& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const opentxs::Identifier& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID) const final;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::string& message) const override;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::string& payment,
        const bool isPayment) const override;
#if OT_CASH
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::shared_ptr<blind::Purse> purse) const override;
#endif
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version) const override;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const OTPeerRequest request,
        const VersionNumber version) const override;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& signerNym,
        const proto::PeerObject& serialized) const override;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const override;
    OTPeerReply PeerReply() const noexcept final;
    OTPeerReply PeerReply(const Nym_p& nym, const proto::PeerReply& serialized)
        const noexcept(false) final;
    OTPeerRequest PeerRequest() const noexcept final;
    OTPeerRequest PeerRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false) final;
    OTZMQPipeline Pipeline(
        std::function<void(opentxs::network::zeromq::Message&)> callback)
        const final;
#if OT_CASH
    std::unique_ptr<blind::Purse> Purse(
        const ServerContext& context,
        const identifier::UnitDefinition& unit,
        const blind::Mint& mint,
        const Amount totalValue,
        const opentxs::PasswordPrompt& reason,
        const proto::CashType type) const final;
    std::unique_ptr<blind::Purse> Purse(
        const proto::Purse& serialized) const final;
    std::unique_ptr<blind::Purse> Purse(
        const identity::Nym& owner,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const opentxs::PasswordPrompt& reason,
        const proto::CashType type) const final;
#endif  // OT_CASH
    OTReplyAcknowledgement ReplyAcknowledgement(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const proto::PeerRequestType type,
        const bool& ack,
        const opentxs::PasswordPrompt& reason) const noexcept(false) final;
    OTReplyAcknowledgement ReplyAcknowledgement(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false) final;
    std::unique_ptr<OTScriptable> Scriptable(
        const String& strCronItem) const final;
    OTSecurityContract SecurityContract(
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) const noexcept(false) final;
    OTSecurityContract SecurityContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false) final;
    OTServerContract ServerContract() const noexcept(false) final;
    OTServerID ServerID() const final;
    OTServerID ServerID(const std::string& serialized) const final;
    OTServerID ServerID(const opentxs::String& serialized) const final;
    std::unique_ptr<OTSignedFile> SignedFile() const final;
    std::unique_ptr<OTSignedFile> SignedFile(
        const String& LOCAL_SUBDIR,
        const String& FILE_NAME) const final;
    std::unique_ptr<OTSignedFile> SignedFile(
        const char* LOCAL_SUBDIR,
        const String& FILE_NAME) const final;
    std::unique_ptr<OTSignedFile> SignedFile(
        const char* LOCAL_SUBDIR,
        const char* FILE_NAME) const final;
    std::unique_ptr<OTSmartContract> SmartContract() const final;
    std::unique_ptr<OTSmartContract> SmartContract(
        const identifier::Server& NOTARY_ID) const final;
    OTStoreSecret StoreSecret(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const proto::SecretType type,
        const std::string& primary,
        const std::string& secondary,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false) final;
    OTStoreSecret StoreSecret(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false) final;
    const api::crypto::Symmetric& Symmetric() const final { return symmetric_; }
    OTSymmetricKey SymmetricKey() const final;
    OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& password,
        const proto::SymmetricMode mode) const final;
    OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized) const final;
    OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const OTPassword& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const proto::SymmetricKeyType type) const final;
    OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const OTPassword& raw,
        const opentxs::PasswordPrompt& reason) const final;
    std::unique_ptr<OTTrade> Trade() const final;
    std::unique_ptr<OTTrade> Trade(
        const identifier::Server& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const opentxs::Identifier& assetAcctId,
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& currencyId,
        const opentxs::Identifier& currencyAcctId) const final;
    std::unique_ptr<OTTransactionType> Transaction(
        const String& strCronItem) const final;
    std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner) const final;
    std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        originType theOriginType = originType::not_applicable) const final;
    std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const final;
    std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        const std::int64_t& lNumberOfOrigin,
        originType theOriginType,
        const std::int64_t& lTransactionNum,
        const std::int64_t& lInRefTo,
        const std::int64_t& lInRefDisplay,
        const Time the_DATE_SIGNED,
        transactionType theType,
        const String& strHash,
        const std::int64_t& lAdjustment,
        const std::int64_t& lDisplayValue,
        const std::int64_t& lClosingNum,
        const std::int64_t& lRequestNum,
        bool bReplyTransSuccess,
        NumList* pNumList = nullptr) const final;
    std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const final;
    std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const final;
    OTUnitID UnitID() const final;
    OTUnitID UnitID(const std::string& serialized) const final;
    OTUnitID UnitID(const opentxs::String& serialized) const final;
    OTUnitDefinition UnitDefinition() const noexcept final;
    OTUnitDefinition UnitDefinition(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false) final;

    ~Factory() override = default;

protected:
    const api::internal::Core& api_;
    std::unique_ptr<const api::crypto::internal::Asymmetric> pAsymmetric_;
    const api::crypto::internal::Asymmetric& asymmetric_;
    std::unique_ptr<const api::crypto::Symmetric> pSymmetric_;
    const api::crypto::Symmetric& symmetric_;

    Factory(const api::internal::Core& api);

private:
    friend opentxs::Factory;

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto instantiate_secp256k1(const ReadView key) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1>;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;
};
}  // namespace opentxs::api::implementation
