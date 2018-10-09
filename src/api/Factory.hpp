// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::implementation
{
class Factory final : public opentxs::api::Factory
{
public:
    std::unique_ptr<opentxs::Basket> Basket() const override;
    std::unique_ptr<opentxs::Basket> Basket(
        std::int32_t nCount,
        std::int64_t lMinimumTransferAmount) const override;

    std::unique_ptr<opentxs::Cheque> Cheque(
        const OTTransaction& receipt) const override;
    std::unique_ptr<opentxs::Cheque> Cheque() const override;
    std::unique_ptr<opentxs::Cheque> Cheque(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const override;

    std::unique_ptr<opentxs::Contract> Contract(
        const String& strCronItem) const override;

    std::unique_ptr<OTCron> Cron(const api::Core& server) const override;

    std::unique_ptr<OTCronItem> CronItem(
        const String& strCronItem) const override;

    std::unique_ptr<opentxs::Item> Item(
        const String& serialized) const override;
    std::unique_ptr<opentxs::Item> Item(
        const std::string& serialized) const override;
    std::unique_ptr<opentxs::Item> Item(
        const Identifier& theNymID,
        const opentxs::Item& theOwner) const override;  // From owner we can get
                                                        // acct ID, server ID,
                                                        // and transaction Num
    std::unique_ptr<opentxs::Item> Item(
        const Identifier& theNymID,
        const OTTransaction& theOwner) const override;  // From owner we can get
                                                        // acct ID, server ID,
                                                        // and transaction Num
    std::unique_ptr<opentxs::Item> Item(
        const Identifier& theNymID,
        const OTTransaction& theOwner,
        itemType theType,
        const Identifier& pDestinationAcctID) const override;
    std::unique_ptr<opentxs::Item> Item(
        const String& strItem,
        const Identifier& theNotaryID,
        std::int64_t lTransactionNumber) const override;
    std::unique_ptr<opentxs::Item> Item(
        const OTTransaction& theOwner,
        itemType theType,
        const Identifier& pDestinationAcctID) const override;

    std::unique_ptr<opentxs::Ledger> Ledger(
        const Identifier& theAccountID,
        const Identifier& theNotaryID) const override;
    std::unique_ptr<opentxs::Ledger> Ledger(
        const Identifier& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID) const override;
    std::unique_ptr<opentxs::Ledger> Ledger(
        const Identifier& theNymID,
        const Identifier& theAcctID,
        const Identifier& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const override;

    std::unique_ptr<OTMarket> Market() const override;
    std::unique_ptr<OTMarket> Market(const char* szFilename) const override;
    virtual std::unique_ptr<OTMarket> Market(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& CURRENCY_TYPE_ID,
        const std::int64_t& lScale) const override;

    std::unique_ptr<opentxs::Message> Message() const override;

#if OT_CASH
    std::unique_ptr<opentxs::Mint> Mint() const override;
    std::unique_ptr<opentxs::Mint> Mint(
        const String& strNotaryID,
        const String& strInstrumentDefinitionID) const override;
    std::unique_ptr<opentxs::Mint> Mint(
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID) const override;
#endif

    std::unique_ptr<OTOffer> Offer() const override;  // The constructor
                                                      // contains the 3
                                                      // variables needed to
                                                      // identify any market.
    std::unique_ptr<OTOffer> Offer(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& CURRENCY_ID,
        const std::int64_t& MARKET_SCALE) const override;

    std::unique_ptr<OTPayment> Payment() const override;
    std::unique_ptr<OTPayment> Payment(const String& strPayment) const override;
    std::unique_ptr<OTPayment> Payment(
        const opentxs::Contract& contract) const override;

#if OT_CRYPTO_WITH_BIP39
    OTPaymentCode PaymentCode(const std::string& base58) const override;
    OTPaymentCode PaymentCode(
        const proto::PaymentCode& serialized) const override;
    OTPaymentCode PaymentCode(
        const std::string& seed,
        const std::uint32_t nym,
        const std::uint8_t version,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const override;
#endif

    std::unique_ptr<OTPaymentPlan> PaymentPlan() const override;
    std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const override;
    std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& SENDER_ACCT_ID,
        const Identifier& SENDER_NYM_ID,
        const Identifier& RECIPIENT_ACCT_ID,
        const Identifier& RECIPIENT_NYM_ID) const override;

#if OT_CASH
    /** just for copy another purse's Server and Instrument Definition Id */
    std::unique_ptr<opentxs::Purse> Purse(
        const opentxs::Purse& thePurse) const override;
    /** similar thing */
    std::unique_ptr<opentxs::Purse> Purse(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const override;
    /** Don't use this unless you really don't know the instrument definition
     * (Like if you're about to read it out of a string.) */
    std::unique_ptr<opentxs::Purse> Purse(
        const Identifier& NOTARY_ID) const override;
    /** Normally you really really want to set the instrument definition. */
    std::unique_ptr<opentxs::Purse> Purse(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& NYM_ID) const override;  // NymID optional
    // OTPayment needs to be able to instantiate OTPurse without knowing the
    // server ID in advance.
    std::unique_ptr<opentxs::Purse> Purse(
        const String& strInput) const override;
    std::unique_ptr<opentxs::Purse> Purse(
        const String& strInput,
        const Identifier& NOTARY_ID) const override;
    std::unique_ptr<opentxs::Purse> Purse(
        const String& strInput,
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const override;
#endif  // OT_CASH

    std::unique_ptr<OTScriptable> Scriptable(
        const String& strCronItem) const override;

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
        const Identifier& NOTARY_ID) const override;

#if OT_CASH
    std::unique_ptr<opentxs::Token> Token(
        const String& strInput) const override;
    std::unique_ptr<opentxs::Token> Token(
        const String& strInput,
        const opentxs::Purse& thePurse) const override;
    std::unique_ptr<opentxs::Token> Token(
        const String& strInput,
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const override;
    std::unique_ptr<opentxs::Token> Token(
        const opentxs::Purse& thePurse,
        const Nym& theNym,
        opentxs::Mint& theMint,
        std::int64_t lDenomination,
        std::int32_t nTokenCount) const override;
#endif

#if OT_CASH_USING_LUCRE
    std::unique_ptr<Token_Lucre> TokenLucre() const override;
    std::unique_ptr<Token_Lucre> TokenLucre(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const override;
    std::unique_ptr<Token_Lucre> TokenLucre(
        const opentxs::Purse& thePurse) const override;
#endif

    std::unique_ptr<OTTrade> Trade() const override;
    std::unique_ptr<OTTrade> Trade(
        const Identifier& notaryID,
        const Identifier& instrumentDefinitionID,
        const Identifier& assetAcctId,
        const Identifier& nymID,
        const Identifier& currencyId,
        const Identifier& currencyAcctId) const override;

    std::unique_ptr<OTTransactionType> Transaction(
        const String& strCronItem) const override;

    std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner) const override;
    std::unique_ptr<OTTransaction> Transaction(
        const Identifier& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
        originType theOriginType = originType::not_applicable) const override;
    std::unique_ptr<OTTransaction> Transaction(
        const Identifier& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const override;
    // THIS factory only used when loading an abbreviated box receipt
    // (inbox, nymbox, or outbox receipt).
    // The full receipt is loaded only after the abbreviated ones are loaded,
    // and verified against them.
    std::unique_ptr<OTTransaction> Transaction(
        const Identifier& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
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
        const Identifier& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const override;
    std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const override;

    ~Factory() = default;

private:
    friend opentxs::Factory;

    const api::Core& api_;

#if OT_CASH
    std::unique_ptr<opentxs::Purse> PurseLowLevel(
        const String& strFirstLine) const override;
    std::unique_ptr<opentxs::Purse> PurseLowLevel(
        const String& strFirstLine,
        const Identifier& NOTARY_ID) const override;
    std::unique_ptr<opentxs::Purse> PurseLowLevel(
        const String& strFirstLine,
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const override;

    std::unique_ptr<opentxs::Token> TokenLowLevel(
        const opentxs::Purse& thePurse) const override;
    std::unique_ptr<opentxs::Token> TokenLowLevel(
        const String& strFirstLine) const override;
    std::unique_ptr<opentxs::Token> TokenLowLevel(
        const String& strFirstLine,
        const opentxs::Purse& thePurse) const override;
    std::unique_ptr<opentxs::Token> TokenLowLevel(
        const String& strFirstLine,
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const override;
#endif  // OT_CASH

    Factory(const api::Core& api);
    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;
};
}  // namespace opentxs::api::implementation
