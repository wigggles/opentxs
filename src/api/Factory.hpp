// Copyright (c) 2018 The Open-Transactions developers
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
    const api::crypto::internal::Asymmetric& Asymmetric() const override
    {
        return asymmetric_;
    }
    OTAsymmetricKey AsymmetricKey(
        const NymParameters& params,
        const proto::KeyRole role,
        const VersionNumber version) const override;
    OTAsymmetricKey AsymmetricKey(
        const proto::AsymmetricKey& serialized,
        const opentxs::PasswordPrompt& reason) const override;
    std::unique_ptr<opentxs::Basket> Basket() const override;
    std::unique_ptr<opentxs::Basket> Basket(
        std::int32_t nCount,
        std::int64_t lMinimumTransferAmount) const override;

    std::unique_ptr<OTPassword> BinarySecret() const override;

    std::unique_ptr<opentxs::Cheque> Cheque(
        const OTTransaction& receipt,
        const opentxs::PasswordPrompt& reason) const override;
    std::unique_ptr<opentxs::Cheque> Cheque() const override;
    std::unique_ptr<opentxs::Cheque> Cheque(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
        const override;

    std::unique_ptr<opentxs::Contract> Contract(
        const String& strCronItem,
        const opentxs::PasswordPrompt& reason) const override;

    std::unique_ptr<OTCron> Cron(const api::Core& server) const override;

    std::unique_ptr<OTCronItem> CronItem(
        const String& strCronItem,
        const opentxs::PasswordPrompt& reason) const override;

    OTIdentifier Identifier() const override;
    OTIdentifier Identifier(const std::string& serialized) const override;
    OTIdentifier Identifier(const opentxs::String& serialized) const override;
    OTIdentifier Identifier(const opentxs::Contract& contract) const override;
    OTIdentifier Identifier(const opentxs::Item& item) const override;

    std::unique_ptr<opentxs::Item> Item(
        const String& serialized,
        const opentxs::PasswordPrompt& reason) const override;
    std::unique_ptr<opentxs::Item> Item(
        const std::string& serialized,
        const opentxs::PasswordPrompt& reason) const override;
    std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const opentxs::Item& theOwner) const override;  // From owner we can get
                                                        // acct ID, server ID,
                                                        // and transaction Num
    std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner) const override;  // From owner we can get
                                                        // acct ID, server ID,
                                                        // and transaction Num
    std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner,
        itemType theType,
        const opentxs::Identifier& pDestinationAcctID) const override;
    std::unique_ptr<opentxs::Item> Item(
        const String& strItem,
        const identifier::Server& theNotaryID,
        std::int64_t lTransactionNumber,
        const opentxs::PasswordPrompt& reason) const override;
    std::unique_ptr<opentxs::Item> Item(
        const OTTransaction& theOwner,
        itemType theType,
        const opentxs::Identifier& pDestinationAcctID) const override;

    OTKeypair Keypair(
        const NymParameters& nymParameters,
        const VersionNumber version,
        const proto::KeyRole role) const override;
    OTKeypair Keypair(
        const api::Core& api,
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey,
        const opentxs::PasswordPrompt& reason) const override;
    OTKeypair Keypair(
        const api::Core& api,
        const proto::AsymmetricKey& serializedPubkey,
        const opentxs::PasswordPrompt& reason) const override;

    std::unique_ptr<opentxs::Ledger> Ledger(
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID) const override;
    std::unique_ptr<opentxs::Ledger> Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID) const override;
    std::unique_ptr<opentxs::Ledger> Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAcctID,
        const identifier::Server& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const override;

    std::unique_ptr<OTMarket> Market() const override;
    std::unique_ptr<OTMarket> Market(const char* szFilename) const override;
    virtual std::unique_ptr<OTMarket> Market(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_TYPE_ID,
        const std::int64_t& lScale) const override;

    std::unique_ptr<opentxs::Message> Message() const override;

#if OT_CASH
    std::unique_ptr<blind::Mint> Mint() const override;
    std::unique_ptr<blind::Mint> Mint(
        const String& strNotaryID,
        const String& strInstrumentDefinitionID) const override;
    std::unique_ptr<blind::Mint> Mint(
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID) const override;
#endif

    OTNymID NymID() const override;
    OTNymID NymID(const std::string& serialized) const override;
    OTNymID NymID(const opentxs::String& serialized) const override;

    std::unique_ptr<OTOffer> Offer() const override;  // The constructor
                                                      // contains the 3
                                                      // variables needed to
                                                      // identify any market.
    std::unique_ptr<OTOffer> Offer(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const std::int64_t& MARKET_SCALE) const override;

    OTPasswordPrompt PasswordPrompt(const std::string& text) const override;

    std::unique_ptr<OTPayment> Payment() const override;
    std::unique_ptr<OTPayment> Payment(const String& strPayment) const override;
    std::unique_ptr<OTPayment> Payment(
        const opentxs::Contract& contract,
        const opentxs::PasswordPrompt& reason) const override;

#if OT_CRYPTO_WITH_BIP39
    OTPaymentCode PaymentCode(
        const std::string& base58,
        const opentxs::PasswordPrompt& reason) const override;
    OTPaymentCode PaymentCode(
        const proto::PaymentCode& serialized,
        const opentxs::PasswordPrompt& reason) const override;
    OTPaymentCode PaymentCode(
        const std::string& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage,
        const std::uint8_t bitmessageVersion,
        const std::uint8_t bitmessageStream) const override;
#endif

    std::unique_ptr<OTPaymentPlan> PaymentPlan() const override;
    std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
        const override;
    std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const opentxs::Identifier& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const opentxs::Identifier& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID) const override;

    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::string& message,
        const opentxs::PasswordPrompt& reason) const override;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::string& payment,
        const bool isPayment,
        const opentxs::PasswordPrompt& reason) const override;
#if OT_CASH
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::shared_ptr<blind::Purse> purse,
        const opentxs::PasswordPrompt& reason) const override;
#endif
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const std::shared_ptr<const PeerRequest> request,
        const std::shared_ptr<const PeerReply> reply,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) const override;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const std::shared_ptr<const PeerRequest> request,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) const override;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& signerNym,
        const proto::PeerObject& serialized,
        const opentxs::PasswordPrompt& reason) const override;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& recipientNym,
        const Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const override;

#if OT_CASH
    std::unique_ptr<blind::Purse> Purse(
        const ServerContext& context,
        const identifier::UnitDefinition& unit,
        const blind::Mint& mint,
        const Amount totalValue,
        const opentxs::PasswordPrompt& reason,
        const proto::CashType type) const override;
    std::unique_ptr<blind::Purse> Purse(
        const proto::Purse& serialized) const override;
    std::unique_ptr<blind::Purse> Purse(
        const identity::Nym& owner,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const opentxs::PasswordPrompt& reason,
        const proto::CashType type) const override;
#endif  // OT_CASH

    std::unique_ptr<OTScriptable> Scriptable(
        const String& strCronItem,
        const opentxs::PasswordPrompt& reason) const override;

    OTServerID ServerID() const override;
    OTServerID ServerID(const std::string& serialized) const override;
    OTServerID ServerID(const opentxs::String& serialized) const override;

    std::unique_ptr<OTSignedFile> SignedFile() const override;
    std::unique_ptr<OTSignedFile> SignedFile(
        const String& LOCAL_SUBDIR,
        const String& FILE_NAME) const override;
    std::unique_ptr<OTSignedFile> SignedFile(
        const char* LOCAL_SUBDIR,
        const String& FILE_NAME) const override;
    std::unique_ptr<OTSignedFile> SignedFile(
        const char* LOCAL_SUBDIR,
        const char* FILE_NAME) const override;

    std::unique_ptr<OTSmartContract> SmartContract() const override;
    std::unique_ptr<OTSmartContract> SmartContract(
        const identifier::Server& NOTARY_ID) const override;

    const api::crypto::Symmetric& Symmetric() const override
    {
        return symmetric_;
    }

    OTSymmetricKey SymmetricKey() const override;
    OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& password,
        const proto::SymmetricMode mode) const override;
    OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized) const override;
    OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const OTPassword& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const proto::SymmetricKeyType type) const override;
    OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const OTPassword& raw,
        const opentxs::PasswordPrompt& reason) const override;

    std::unique_ptr<OTTrade> Trade() const override;
    std::unique_ptr<OTTrade> Trade(
        const identifier::Server& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const opentxs::Identifier& assetAcctId,
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& currencyId,
        const opentxs::Identifier& currencyAcctId) const override;

    std::unique_ptr<OTTransactionType> Transaction(
        const String& strCronItem,
        const opentxs::PasswordPrompt& reason) const override;

    std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner) const override;
    std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        originType theOriginType = originType::not_applicable) const override;
    std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const override;
    // THIS factory only used when loading an abbreviated box receipt
    // (inbox, nymbox, or outbox receipt).
    // The full receipt is loaded only after the abbreviated ones are loaded,
    // and verified against them.
    std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        const std::int64_t& lNumberOfOrigin,
        originType theOriginType,
        const std::int64_t& lTransactionNum,
        const std::int64_t& lInRefTo,
        const std::int64_t& lInRefDisplay,
        time64_t the_DATE_SIGNED,
        transactionType theType,
        const String& strHash,
        const std::int64_t& lAdjustment,
        const std::int64_t& lDisplayValue,
        const std::int64_t& lClosingNum,
        const std::int64_t& lRequestNum,
        bool bReplyTransSuccess,
        NumList* pNumList = nullptr) const override;
    std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const override;
    std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const override;

    OTUnitID UnitID() const override;
    OTUnitID UnitID(const std::string& serialized) const override;
    OTUnitID UnitID(const opentxs::String& serialized) const override;

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

    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;
};
}  // namespace opentxs::api::implementation
