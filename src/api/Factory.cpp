// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/Factory.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#endif
#include "opentxs/cash/Mint.hpp"
#include "opentxs/cash/MintLucre.hpp"
#include "opentxs/cash/Purse.hpp"
#include "opentxs/cash/Token.hpp"
#include "opentxs/cash/TokenLucre.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/contract/basket/Basket.hpp"
#include "opentxs/core/crypto/OTSignedFile.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/recurring/OTAgreement.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/trade/OTTrade.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include "Factory.hpp"

//#define OT_METHOD "opentxs::api::implementation::Factory::"

namespace opentxs
{
api::Factory* Factory::FactoryAPI(
#if OT_CRYPTO_WITH_BIP39
    const api::HDSeed& seeds
#endif
)
{
    return new api::implementation::Factory(
#if OT_CRYPTO_WITH_BIP39
        seeds
#endif
    );
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
Factory::Factory(
#if OT_CRYPTO_WITH_BIP39
    const api::HDSeed& seeds
#endif
    )
    :
#if OT_CRYPTO_WITH_BIP39
    seeds_(seeds)
#endif
{
}

std::unique_ptr<opentxs::Basket> Factory::Basket(const api::Core& core) const
{
    std::unique_ptr<opentxs::Basket> basket;
    basket.reset(new opentxs::Basket(core));

    return basket;
}

std::unique_ptr<opentxs::Basket> Factory::Basket(
    const api::Core& core,
    std::int32_t nCount,
    std::int64_t lMinimumTransferAmount) const
{
    std::unique_ptr<opentxs::Basket> basket;
    basket.reset(new opentxs::Basket(core, nCount, lMinimumTransferAmount));

    return basket;
}

std::unique_ptr<opentxs::Cheque> Factory::Cheque(
    const OTTransaction& receipt) const
{
    std::unique_ptr<opentxs::Cheque> output{
        new opentxs::Cheque{receipt.Core()}};

    OT_ASSERT(output)

    String serializedItem{};
    receipt.GetReferenceString(serializedItem);
    std::unique_ptr<opentxs::Item> item{Item(
        receipt.Core(),
        serializedItem,
        receipt.GetRealNotaryID(),
        receipt.GetReferenceToNum())};

    OT_ASSERT(false != bool(item));

    String serializedCheque{};
    item->GetAttachment(serializedCheque);
    const auto loaded = output->LoadContractFromString(serializedCheque);

    if (false == loaded) {
        otErr << __FUNCTION__ << ": Failed to load cheque," << std::endl;
    }

    return output;
}

std::unique_ptr<opentxs::Cheque> Factory::Cheque(const api::Core& core) const
{
    std::unique_ptr<opentxs::Cheque> cheque;
    cheque.reset(new opentxs::Cheque(core));

    return cheque;
}

std::unique_ptr<opentxs::Cheque> Factory::Cheque(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    std::unique_ptr<opentxs::Cheque> cheque;
    cheque.reset(
        new opentxs::Cheque(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return cheque;
}

std::unique_ptr<opentxs::Contract> Factory::Contract(
    const opentxs::api::Core& core,
    const opentxs::String& strInput) const
{

    using namespace opentxs;
    String strContract, strFirstLine;  // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {

        std::unique_ptr<opentxs::Contract> pContract;

        if (strFirstLine.Contains(
                "-----BEGIN SIGNED SMARTCONTRACT-----"))  // this string is 36
                                                          // chars long.
        {
            pContract.reset(new OTSmartContract(core));
            OT_ASSERT(false != bool(pContract));
        }

        if (strFirstLine.Contains(
                "-----BEGIN SIGNED PAYMENT PLAN-----"))  // this string is 35
                                                         // chars long.
        {
            pContract.reset(new OTPaymentPlan(core));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine.Contains(
                       "-----BEGIN SIGNED TRADE-----"))  // this string is 28
                                                         // chars long.
        {
            pContract.reset(new OTTrade(core));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine.Contains("-----BEGIN SIGNED OFFER-----")) {
            pContract.reset(new OTOffer(core));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine.Contains("-----BEGIN SIGNED INVOICE-----")) {
            pContract.reset(new opentxs::Cheque(core));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine.Contains("-----BEGIN SIGNED VOUCHER-----")) {
            pContract.reset(new opentxs::Cheque(core));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine.Contains("-----BEGIN SIGNED CHEQUE-----")) {
            pContract.reset(new opentxs::Cheque(core));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine.Contains("-----BEGIN SIGNED MESSAGE-----")) {
            pContract.reset(new opentxs::Message(core));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine.Contains("-----BEGIN SIGNED MINT-----")) {
#if OT_CASH
            auto mint = Mint(core);
            pContract.reset(mint.release());
            OT_ASSERT(false != bool(pContract));
#endif  // OT_CASH
        } else if (strFirstLine.Contains("-----BEGIN SIGNED FILE-----")) {
            //            pContract.reset(new OTSignedFile(core));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine.Contains("-----BEGIN SIGNED CASH-----")) {
#if OT_CASH
            auto token = TokenLowLevel(core, strFirstLine);
            pContract.reset(token.release());
            OT_ASSERT(false != bool(pContract));
#endif  // OT_CASH
        } else if (strFirstLine.Contains("-----BEGIN SIGNED CASH TOKEN-----")) {
#if OT_CASH
            auto token = TokenLowLevel(core, strFirstLine);
            pContract.reset(token.release());
            OT_ASSERT(false != bool(pContract));
#endif  // OT_CASH
        } else if (strFirstLine.Contains(
                       "-----BEGIN SIGNED LUCRE CASH TOKEN-----")) {
#if OT_CASH
            auto token = TokenLowLevel(core, strFirstLine);
            pContract.reset(token.release());
            OT_ASSERT(false != bool(pContract));
#endif  // OT_CASH
        }

        // The string didn't match any of the options in the factory.
        //
        if (!pContract) {
            otOut << __FUNCTION__
                  << ": Object type not yet supported by class factory: "
                  << strFirstLine << "\n";
            // Does the contract successfully load from the string passed in?
        } else if (!pContract->LoadContractFromString(strContract)) {
            otOut << __FUNCTION__
                  << ": Failed loading contract from string (first line): "
                  << strFirstLine << "\n";
        } else {
            return pContract;
        }
    }
    return nullptr;
}

std::unique_ptr<OTCron> Factory::Cron(const api::Core& server) const
{
    std::unique_ptr<opentxs::OTCron> cron;
    cron.reset(new opentxs::OTCron(server));

    return cron;
}

std::unique_ptr<OTCronItem> Factory::CronItem(
    const api::Core& core,
    const String& strCronItem) const
{
    static char buf[45] = "";

    if (!strCronItem.Exists()) {
        otErr << __FUNCTION__
              << ": Empty string was passed in (returning nullptr.)\n";
        return nullptr;
    }

    String strContract(strCronItem);

    if (!strContract.DecodeIfArmored(false)) {
        otErr << __FUNCTION__
              << ": Input string apparently was encoded and "
                 "then failed decoding. Contents: \n"
              << strCronItem << "\n";
        return nullptr;
    }

    strContract.reset();  // for sgets
    buf[0] = 0;           // probably unnecessary.
    bool bGotLine = strContract.sgets(buf, 40);

    if (!bGotLine) return nullptr;

    String strFirstLine(buf);
    // set the "file" pointer within this string back to index 0.
    strContract.reset();

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 45 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine.Exists() || strFirstLine.Contains("- -")) return nullptr;

    // By this point we know already that it's not escaped.
    // BUT it might still be ARMORED!

    std::unique_ptr<OTCronItem> pItem;
    // this string is 35 chars long.
    if (strFirstLine.Contains("-----BEGIN SIGNED PAYMENT PLAN-----")) {
        pItem.reset(new OTPaymentPlan(core));
    }
    // this string is 28 chars long.
    else if (strFirstLine.Contains("-----BEGIN SIGNED TRADE-----")) {
        pItem.reset(new OTTrade(core));
    }
    // this string is 36 chars long.
    else if (strFirstLine.Contains("-----BEGIN SIGNED SMARTCONTRACT-----")) {
        pItem.reset(new OTSmartContract(core));
    } else {
        return nullptr;
    }

    // Does the contract successfully load from the string passed in?
    if (pItem->LoadContractFromString(strContract)) { return pItem; }

    return nullptr;
}

std::unique_ptr<opentxs::Item> Factory::Item(
    const api::Core& core,
    const Identifier& theNymID,
    const opentxs::Item& theOwner) const
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(core, theNymID, theOwner));

    return item;
}

std::unique_ptr<opentxs::Item> Factory::Item(
    const api::Core& core,
    const Identifier& theNymID,
    const OTTransaction& theOwner) const
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(core, theNymID, theOwner));

    return item;
}

std::unique_ptr<opentxs::Item> Factory::Item(
    const api::Core& core,
    const Identifier& theNymID,
    const OTTransaction& theOwner,
    itemType theType,
    const Identifier& pDestinationAcctID) const
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(
        core, theNymID, theOwner, theType, pDestinationAcctID));

    return item;
}

// Sometimes I don't know user ID of the originator, or the account ID of the
// originator,
// until after I have loaded the item. It's simply impossible to set those
// values ahead
// of time, sometimes. In those cases, we set the values appropriately but then
// we need
// to verify that the user ID is actually the owner of the AccountID. TOdo that.
std::unique_ptr<opentxs::Item> Factory::Item(
    const api::Core& core,
    const String& strItem,
    const Identifier& theNotaryID,
    std::int64_t lTransactionNumber) const
{
    if (!strItem.Exists()) {
        otErr << "Item::CreateItemFromString: strItem is empty. (Expected an "
                 "item.)\n";
        return nullptr;
    }

    std::unique_ptr<opentxs::Item> pItem{new opentxs::Item(core)};

    // So when it loads its own server ID, we can compare to this one.
    pItem->SetRealNotaryID(theNotaryID);

    // This loads up the purported account ID and the user ID.
    if (pItem->LoadContractFromString(strItem)) {
        const Identifier& ACCOUNT_ID = pItem->GetPurportedAccountID();
        pItem->SetRealAccountID(ACCOUNT_ID);  // I do this because it's all
                                              // we've got in this case. It's
                                              // what's in the
        // xml, so it must be right. If it's a lie, the signature will fail or
        // the
        // user will not show as the owner of that account. But remember, the
        // server
        // sent the message in the first place.

        pItem->SetTransactionNum(lTransactionNumber);

        if (pItem->VerifyContractID())  // this compares purported and real
                                        // account IDs, as well as server IDs.
        {
            return pItem;
        }
    }

    return nullptr;
}

// Let's say you have created a transaction, and you are creating an item to put
// into it.
// Well in that case, you don't care to verify that the real IDs match the
// purported IDs, since
// you are creating this item yourself, not verifying it from someone else.
// Use this function to create the new Item before you add it to your new
// Transaction.
std::unique_ptr<opentxs::Item> Factory::Item(
    const OTTransaction& theOwner,
    itemType theType,
    const Identifier& pDestinationAcctID) const
{
    std::unique_ptr<opentxs::Item> pItem{new opentxs::Item(
        theOwner.Core(),
        theOwner.GetNymID(),
        theOwner,
        theType,
        pDestinationAcctID)};

    if (false != bool(pItem)) {
        pItem->SetPurportedAccountID(theOwner.GetPurportedAccountID());
        pItem->SetPurportedNotaryID(theOwner.GetPurportedNotaryID());
        return pItem;
    }
    return nullptr;
}

std::unique_ptr<opentxs::Ledger> Factory::Ledger(
    const api::Core& core,
    const Identifier& theAccountID,
    const Identifier& theNotaryID) const
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(new opentxs::Ledger(core, theAccountID, theNotaryID));

    return ledger;
}

std::unique_ptr<opentxs::Ledger> Factory::Ledger(
    const api::Core& core,
    const Identifier& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID) const
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(
        new opentxs::Ledger(core, theNymID, theAccountID, theNotaryID));

    return ledger;
}

std::unique_ptr<opentxs::Ledger> Factory::Ledger(
    const api::Core& core,
    const Identifier& theNymID,
    const Identifier& theAcctID,
    const Identifier& theNotaryID,
    ledgerType theType,
    bool bCreateFile) const
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(new opentxs::Ledger(core, theNymID, theAcctID, theNotaryID));

    ledger->generate_ledger(
        theNymID, theAcctID, theNotaryID, theType, bCreateFile);

    return ledger;
}

std::unique_ptr<OTMarket> Factory::Market(const api::Core& core) const
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(core));

    return market;
}

std::unique_ptr<OTMarket> Factory::Market(
    const api::Core& core,
    const char* szFilename) const
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(core, szFilename));

    return market;
}

std::unique_ptr<OTMarket> Factory::Market(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& CURRENCY_TYPE_ID,
    const std::int64_t& lScale) const
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(
        core, NOTARY_ID, INSTRUMENT_DEFINITION_ID, CURRENCY_TYPE_ID, lScale));

    return market;
}

std::unique_ptr<opentxs::Message> Factory::Message(const api::Core& core) const
{
    std::unique_ptr<opentxs::Message> message;
    message.reset(new opentxs::Message(core));

    return message;
}

std::unique_ptr<opentxs::Mint> Factory::Mint(const api::Core& core) const
{
    std::unique_ptr<opentxs::Mint> pMint;

#if OT_CASH_USING_LUCRE
    pMint.reset(new MintLucre(core));
    OT_ASSERT(false != bool(pMint));
#elif OT_CASH_USING_MAGIC_MONEY
    otErr << __FUCNTION__
          << ": Open-Transactions doesn't support Magic Money "
             "by Pr0duct Cypher (yet), "
          << "so it's impossible to instantiate a mint.\n";
#else
    otErr
        << __FUNCTION__
        << ": Open-Transactions isn't built with any digital cash algorithms, "
        << "so it's impossible to instantiate a mint.\n";
#endif
    return pMint;
}

std::unique_ptr<opentxs::Mint> Factory::Mint(
    const api::Core& core,
    const String& strNotaryID,
    const String& strInstrumentDefinitionID) const
{
    std::unique_ptr<opentxs::Mint> pMint;

#if OT_CASH_USING_LUCRE
    pMint.reset(new MintLucre(core, strNotaryID, strInstrumentDefinitionID));
    OT_ASSERT(false != bool(pMint));
#elif OT_CASH_USING_MAGIC_MONEY
    otErr << __FUNCTION__
          << ": Open-Transactions doesn't support Magic Money "
             "by Pr0duct Cypher (yet), "
          << "so it's impossible to instantiate a mint.\n";
#else
    otErr
        << __FUNCTION__
        << ": Open-Transactions isn't built with any digital cash algorithms, "
        << "so it's impossible to instantiate a mint.\n";
#endif
    return pMint;
}

std::unique_ptr<opentxs::Mint> Factory::Mint(
    const api::Core& core,
    const String& strNotaryID,
    const String& strServerNymID,
    const String& strInstrumentDefinitionID) const
{
    std::unique_ptr<opentxs::Mint> pMint;

#if OT_CASH_USING_LUCRE
    pMint.reset(new MintLucre(
        core, strNotaryID, strServerNymID, strInstrumentDefinitionID));
    OT_ASSERT(false != bool(pMint));
#elif OT_CASH_USING_MAGIC_MONEY
    otErr << __FUNCTION__
          << ": Open-Transactions doesn't support Magic Money "
             "by Pr0duct Cypher (yet), "
          << "so it's impossible to instantiate a mint.\n";
#else
    otErr
        << __FUNCTION__
        << ": Open-Transactions isn't built with any digital cash algorithms, "
        << "so it's impossible to instantiate a mint.\n";
#endif
    return pMint;
}

std::unique_ptr<OTOffer> Factory::Offer(const api::Core& core) const
{
    std::unique_ptr<OTOffer> offer;
    offer.reset(new OTOffer(core));

    return offer;
}

std::unique_ptr<OTOffer> Factory::Offer(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& CURRENCY_ID,
    const std::int64_t& MARKET_SCALE) const
{
    std::unique_ptr<OTOffer> offer;
    offer.reset(new OTOffer(
        core, NOTARY_ID, INSTRUMENT_DEFINITION_ID, CURRENCY_ID, MARKET_SCALE));

    return offer;
}

std::unique_ptr<OTPayment> Factory::Payment(const api::Core& core) const
{
    std::unique_ptr<OTPayment> payment;
    payment.reset(new OTPayment(core));

    return payment;
}

std::unique_ptr<OTPayment> Factory::Payment(
    const api::Core& core,
    const String& strPayment) const
{
    std::unique_ptr<OTPayment> payment;
    payment.reset(new OTPayment(core, strPayment));

    return payment;
}

#if OT_CRYPTO_WITH_BIP39
OTPaymentCode Factory::PaymentCode(const std::string& base58) const
{
    return opentxs::PaymentCode::Factory(seeds_, base58);
}

OTPaymentCode Factory::PaymentCode(const proto::PaymentCode& serialized) const
{
    return opentxs::PaymentCode::Factory(seeds_, serialized);
}

OTPaymentCode Factory::PaymentCode(
    const std::string& seed,
    const std::uint32_t nym,
    const std::uint8_t version,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream) const
{
    return opentxs::PaymentCode::Factory(
        seeds_,
        seed,
        nym,
        version,
        bitmessage,
        bitmessageVersion,
        bitmessageStream);
}
#endif

std::unique_ptr<OTPaymentPlan> Factory::PaymentPlan(const api::Core& core) const
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(new OTPaymentPlan(core));

    return paymentplan;
}

std::unique_ptr<OTPaymentPlan> Factory::PaymentPlan(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(
        new OTPaymentPlan(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return paymentplan;
}

std::unique_ptr<OTPaymentPlan> Factory::PaymentPlan(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& SENDER_ACCT_ID,
    const Identifier& SENDER_NYM_ID,
    const Identifier& RECIPIENT_ACCT_ID,
    const Identifier& RECIPIENT_NYM_ID) const
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(new OTPaymentPlan(
        core,
        NOTARY_ID,
        INSTRUMENT_DEFINITION_ID,
        SENDER_ACCT_ID,
        SENDER_NYM_ID,
        RECIPIENT_ACCT_ID,
        RECIPIENT_NYM_ID));

    return paymentplan;
}

#if OT_CASH
std::unique_ptr<opentxs::Purse> Factory::Purse(
    const api::Core& core,
    const opentxs::Purse& thePurse) const
{
    std::unique_ptr<opentxs::Purse> purse;
    purse.reset(new opentxs::Purse(core, thePurse));

    return purse;
}

std::unique_ptr<opentxs::Purse> Factory::Purse(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    std::unique_ptr<opentxs::Purse> purse;
    purse.reset(new opentxs::Purse(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return purse;
}

std::unique_ptr<opentxs::Purse> Factory::Purse(
    const api::Core& core,
    const Identifier& NOTARY_ID) const
{
    std::unique_ptr<opentxs::Purse> purse;
    purse.reset(new opentxs::Purse(core, NOTARY_ID));

    return purse;
}

std::unique_ptr<opentxs::Purse> Factory::Purse(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& NYM_ID) const
{
    std::unique_ptr<opentxs::Purse> purse;
    purse.reset(
        new opentxs::Purse(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID, NYM_ID));

    return purse;
}

std::unique_ptr<opentxs::Purse> Factory::Purse(
    const opentxs::Purse& thePurse) const
{
    std::unique_ptr<opentxs::Purse> purse;
    purse.reset(new opentxs::Purse(thePurse));

    return purse;
}

std::unique_ptr<opentxs::Purse> Factory::Purse(
    const api::Core& core,
    String strInput) const
{
    String strContract, strFirstLine;  // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        auto pPurse = PurseLowLevel(core, strFirstLine);

        // The string didn't match any of the options in the factory.
        if (false == bool(pPurse)) return nullptr;

        // Does the contract successfully load from the string passed in?
        if (pPurse->LoadContractFromString(strContract)) return pPurse;
    }

    return nullptr;
}

std::unique_ptr<opentxs::Purse> Factory::Purse(
    const api::Core& core,
    String strInput,
    const Identifier& NOTARY_ID) const
{
    String strContract, strFirstLine;  // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        auto pPurse = PurseLowLevel(core, strFirstLine, NOTARY_ID);

        // The string didn't match any of the options in the factory.
        if (false == bool(pPurse)) return nullptr;

        // Does the contract successfully load from the string passed in?
        if (pPurse->LoadContractFromString(strContract)) {
            if (NOTARY_ID != pPurse->GetNotaryID()) {
                const String strNotaryID(NOTARY_ID),
                    strPurseNotaryID(pPurse->GetNotaryID());
                otErr << "Purse::PurseFactory"
                      << ": Failure: NotaryID on purse (" << strPurseNotaryID
                      << ") doesn't match expected server ID (" << strNotaryID
                      << ").\n";
            } else
                return pPurse;
        }
    }

    return nullptr;
}

std::unique_ptr<opentxs::Purse> Factory::Purse(
    const api::Core& core,
    String strInput,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    String strContract, strFirstLine;  // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        auto pPurse = PurseLowLevel(
            core, strFirstLine, NOTARY_ID, INSTRUMENT_DEFINITION_ID);

        // The string didn't match any of the options in the factory.
        if (false == bool(pPurse)) return nullptr;

        // Does the contract successfully load from the string passed in?
        if (pPurse->LoadContractFromString(strContract)) {
            const char* szFunc = "Purse::PurseFactory";
            if (NOTARY_ID != pPurse->GetNotaryID()) {
                const String strNotaryID(NOTARY_ID),
                    strPurseNotaryID(pPurse->GetNotaryID());
                otErr << szFunc << ": Failure: NotaryID on purse ("
                      << strPurseNotaryID
                      << ") doesn't match expected "
                         "server ID ("
                      << strNotaryID << ").\n";
            } else if (
                INSTRUMENT_DEFINITION_ID !=
                pPurse->GetInstrumentDefinitionID()) {
                const String strInstrumentDefinitionID(
                    INSTRUMENT_DEFINITION_ID),
                    strPurseInstrumentDefinitionID(
                        pPurse->GetInstrumentDefinitionID());
                otErr << szFunc
                      << ": Failure: InstrumentDefinitionID on purse ("
                      << strPurseInstrumentDefinitionID
                      << ") doesn't match expected "
                         "instrument definition id ("
                      << strInstrumentDefinitionID << ").\n";
            } else
                return pPurse;
        }
    }

    return nullptr;
}

std::unique_ptr<opentxs::Purse> Factory::PurseLowLevel(
    const api::Core& core,
    const String& strFirstLine) const
{
    if (strFirstLine.Contains("-----BEGIN SIGNED PURSE-----"))  // this string
                                                                // is
    // 28 chars long.
    // todo
    // hardcoding.
    {
        std::unique_ptr<opentxs::Purse> pPurse{new opentxs::Purse(core)};
        OT_ASSERT(false != bool(pPurse));
        return pPurse;
    }
    return nullptr;
}

std::unique_ptr<opentxs::Purse> Factory::PurseLowLevel(
    const api::Core& core,
    const String& strFirstLine,
    const Identifier& NOTARY_ID) const
{
    if (strFirstLine.Contains("-----BEGIN SIGNED PURSE-----"))  // this string
                                                                // is
    // 28 chars long.
    // todo
    // hardcoding.
    {
        std::unique_ptr<opentxs::Purse> pPurse{
            new opentxs::Purse(core, NOTARY_ID)};
        OT_ASSERT(false != bool(pPurse));
        return pPurse;
    }
    return nullptr;
}

std::unique_ptr<opentxs::Purse> Factory::PurseLowLevel(
    const api::Core& core,
    const String& strFirstLine,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    if (strFirstLine.Contains("-----BEGIN SIGNED PURSE-----"))  // this string
                                                                // is
    // 28 chars long.
    // todo
    // hardcoding.
    {
        std::unique_ptr<opentxs::Purse> pPurse{
            new opentxs::Purse(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID)};
        OT_ASSERT(false != bool(pPurse));
        return pPurse;
    }
    return nullptr;
}
#endif  // OT_CASH

std::unique_ptr<OTScriptable> Factory::Scriptable(
    const api::Core& core,
    const String& strInput) const
{
    static char buf[45] = "";

    if (!strInput.Exists()) {
        otErr << __FUNCTION__ << ": Failure: Input string is empty.\n";
        return nullptr;
    }

    String strContract(strInput);

    if (!strContract.DecodeIfArmored(false))  // bEscapedIsAllowed=true
                                              // by default.
    {
        otErr << __FUNCTION__
              << ": Input string apparently was encoded and "
                 "then failed decoding. Contents: \n"
              << strInput << "\n";
        return nullptr;
    }

    // At this point, strContract contains the actual contents, whether they
    // were originally ascii-armored OR NOT. (And they are also now trimmed,
    // either way.)
    //
    strContract.reset();  // for sgets
    buf[0] = 0;           // probably unnecessary.
    bool bGotLine = strContract.sgets(buf, 40);

    if (!bGotLine) return nullptr;

    std::unique_ptr<OTScriptable> pItem;

    String strFirstLine(buf);
    strContract.reset();  // set the "file" pointer within this string back to
                          // index 0.

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 45 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine.Exists() || strFirstLine.Contains("- -")) return nullptr;

    // There are actually two factories that load smart contracts. See
    // OTCronItem.
    //
    else if (strFirstLine.Contains(
                 "-----BEGIN SIGNED SMARTCONTRACT-----"))  // this string is 36
                                                           // chars long.
    {
        pItem.reset(new OTSmartContract(core));
        OT_ASSERT(false != bool(pItem));
    }

    // The string didn't match any of the options in the factory.
    if (false == bool(pItem)) return nullptr;

    // Does the contract successfully load from the string passed in?
    if (pItem->LoadContractFromString(strContract)) return pItem;

    return nullptr;
}

std::unique_ptr<OTSignedFile> Factory::SignedFile(const api::Core& core) const
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(core));

    return signedfile;
}

std::unique_ptr<OTSignedFile> Factory::SignedFile(
    const api::Core& core,
    const String& LOCAL_SUBDIR,
    const String& FILE_NAME) const
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(core, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}
std::unique_ptr<OTSignedFile> Factory::SignedFile(
    const api::Core& core,
    const char* LOCAL_SUBDIR,
    const String& FILE_NAME) const
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(core, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}

std::unique_ptr<OTSignedFile> Factory::SignedFile(
    const api::Core& core,
    const char* LOCAL_SUBDIR,
    const char* FILE_NAME) const
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(core, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}

std::unique_ptr<OTSmartContract> Factory::SmartContract(
    const api::Core& core) const
{
    std::unique_ptr<OTSmartContract> smartcontract;
    smartcontract.reset(new OTSmartContract(core));

    return smartcontract;
}

std::unique_ptr<OTSmartContract> Factory::SmartContract(
    const api::Core& core,
    const Identifier& NOTARY_ID) const
{
    std::unique_ptr<OTSmartContract> smartcontract;
    smartcontract.reset(new OTSmartContract(core, NOTARY_ID));

    return smartcontract;
}

#if OT_CASH
// static -- class factory.
//
std::unique_ptr<opentxs::Token> Factory::Token(
    const api::Core& core,
    String strInput,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    String strContract, strFirstLine;  // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        std::unique_ptr<opentxs::Token> pToken = TokenLowLevel(
            core, strFirstLine, NOTARY_ID, INSTRUMENT_DEFINITION_ID);

        // The string didn't match any of the options in the factory.
        if (false == bool(pToken)) return nullptr;

        // Does the contract successfully load from the string passed in?
        if (pToken->LoadContractFromString(strContract)) return pToken;
    }

    return nullptr;
}

std::unique_ptr<opentxs::Token> Factory::Token(
    const api::Core& core,
    String strInput) const
{
    String strContract, strFirstLine;  // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        std::unique_ptr<opentxs::Token> pToken =
            TokenLowLevel(core, strFirstLine);

        // The string didn't match any of the options in the factory.
        if (false == bool(pToken)) return nullptr;

        // Does the contract successfully load from the string passed in?
        if (pToken->LoadContractFromString(strContract)) return pToken;
    }

    return nullptr;
}

std::unique_ptr<opentxs::Token> Factory::Token(
    String strInput,
    const opentxs::Purse& thePurse) const
{
    String strContract, strFirstLine;  // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        std::unique_ptr<opentxs::Token> pToken =
            TokenLowLevel(strFirstLine, thePurse);

        // The string didn't match any of the options in the factory.
        if (false == bool(pToken)) return nullptr;

        // Does the contract successfully load from the string passed in?
        if (pToken->LoadContractFromString(strContract)) return pToken;
    }

    return nullptr;
}

std::unique_ptr<Token> Factory::Token(
    const opentxs::Purse& thePurse,
    const Nym& theNym,
    opentxs::Mint& theMint,
    std::int64_t lDenomination,
    std::int32_t nTokenCount) const
{
    auto pToken{TokenLowLevel(thePurse)};  // already asserts.
    OT_ASSERT(false != bool(pToken));      // Just for good measure.

    const bool bGeneratedRequest = pToken->GenerateTokenRequest(
        theNym, theMint, lDenomination, nTokenCount);

    if (!bGeneratedRequest) {
        otErr << __FUNCTION__ << ": Failed trying to generate token request.\n";
    }

    return pToken;
}

std::unique_ptr<opentxs::Token> Factory::TokenLowLevel(
    const api::Core& core,
    const String& strFirstLine,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    std::unique_ptr<opentxs::Token> pToken;

#if OT_CASH_USING_LUCRE
    if (strFirstLine.Contains("-----BEGIN SIGNED CASH-----"))  // this string is
                                                               // 27 chars long.
    {
        pToken.reset(
            new Token_Lucre(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID));
        OT_ASSERT(false != bool(pToken));
    } else if (strFirstLine.Contains(
                   "-----BEGIN SIGNED CASH TOKEN-----"))  // this string is 33
                                                          // chars long.
    {
        pToken.reset(
            new Token_Lucre(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID));
        OT_ASSERT(false != bool(pToken));
    } else if (strFirstLine.Contains(
                   "-----BEGIN SIGNED LUCRE CASH TOKEN-----"))  // this string
                                                                // is
    // 39 chars long.
    {
        pToken.reset(
            new Token_Lucre(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID));
        OT_ASSERT(false != bool(pToken));
    }
#else
    otErr << __FUNCTION__
          << ": Open-Transactions is not built for any digital "
             "cash algorithms. (Failure.)";
#endif  // OT_CASH_USING_LUCRE

    return pToken;
}

std::unique_ptr<opentxs::Token> Factory::TokenLowLevel(
    const String& strFirstLine,
    const opentxs::Purse& thePurse) const
{
    std::unique_ptr<opentxs::Token> pToken;

#if OT_CASH_USING_LUCRE
    if (strFirstLine.Contains("-----BEGIN SIGNED CASH-----"))  // this string is
                                                               // 27 chars long.
    {
        pToken.reset(new Token_Lucre(thePurse.Core(), thePurse));
        OT_ASSERT(false != bool(pToken));
    } else if (strFirstLine.Contains(
                   "-----BEGIN SIGNED CASH TOKEN-----"))  // this string is 33
                                                          // chars long.
    {
        pToken.reset(new Token_Lucre(thePurse.Core(), thePurse));
        OT_ASSERT(false != bool(pToken));
    } else if (strFirstLine.Contains(
                   "-----BEGIN SIGNED LUCRE CASH TOKEN-----"))  // this string
                                                                // is
    // 39 chars long.
    {
        pToken.reset(new Token_Lucre(thePurse.Core(), thePurse));
        OT_ASSERT(false != bool(pToken));
    }
#else
    otErr << __FUNCTION__
          << ": Open-Transactions is not built for any digital "
             "cash algorithms. (Failure.)";
#endif  // OT_CASH_USING_LUCRE

    return pToken;
}

std::unique_ptr<opentxs::Token> Factory::TokenLowLevel(
    const opentxs::Purse& thePurse) const
{
    std::unique_ptr<opentxs::Token> pToken;

#if OT_CASH_USING_LUCRE
    pToken.reset(new Token_Lucre(thePurse.Core(), thePurse));
    OT_ASSERT(false != bool(pToken));
#else
    otErr << __FUNCTION__
          << ": Open-Transactions is not built for any digital "
             "cash algorithms. (Failure.)";
#endif  // OT_CASH_USING_LUCRE

    return pToken;
}

std::unique_ptr<opentxs::Token> Factory::TokenLowLevel(
    const api::Core& core,
    const String& strFirstLine) const
{
    std::unique_ptr<opentxs::Token> pToken;

#if OT_CASH_USING_LUCRE
    if (strFirstLine.Contains("-----BEGIN SIGNED CASH-----"))  // this string is
                                                               // 27 chars long.
    {
        pToken.reset(new Token_Lucre(core));
        OT_ASSERT(false != bool(pToken));
    } else if (strFirstLine.Contains(
                   "-----BEGIN SIGNED CASH TOKEN-----"))  // this string is 33
                                                          // chars long.
    {
        pToken.reset(new Token_Lucre(core));
        OT_ASSERT(false != bool(pToken));
    } else if (strFirstLine.Contains(
                   "-----BEGIN SIGNED LUCRE CASH TOKEN-----"))  // this string
                                                                // is
    // 39 chars long.
    {
        pToken.reset(new Token_Lucre(core));
        OT_ASSERT(false != bool(pToken));
    }
#else
    otErr << __FUNCTION__
          << ": Open-Transactions is not built for any digital "
             "cash algorithms. (Failure.)";
#endif  // OT_CASH_USING_LUCRE

    return pToken;
}
#endif  // OT_CASH

#if OT_CASH_USING_LUCRE
std::unique_ptr<Token_Lucre> Factory::TokenLucre(const api::Core& core) const
{
    std::unique_ptr<Token_Lucre> token;
    token.reset(new Token_Lucre(core));

    return token;
}

std::unique_ptr<Token_Lucre> Factory::TokenLucre(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    std::unique_ptr<Token_Lucre> token;
    token.reset(new Token_Lucre(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return token;
}

std::unique_ptr<Token_Lucre> Factory::TokenLucre(
    const api::Core& core,
    const opentxs::Purse& thePurse) const
{
    std::unique_ptr<Token_Lucre> token;
    token.reset(new Token_Lucre(core, thePurse));

    return token;
}
#endif

std::unique_ptr<OTTrade> Factory::Trade(const api::Core& core) const
{
    std::unique_ptr<OTTrade> trade;
    trade.reset(new OTTrade(core));

    return trade;
}

std::unique_ptr<OTTrade> Factory::Trade(
    const api::Core& core,
    const Identifier& notaryID,
    const Identifier& instrumentDefinitionID,
    const Identifier& assetAcctId,
    const Identifier& nymID,
    const Identifier& currencyId,
    const Identifier& currencyAcctId) const
{
    std::unique_ptr<OTTrade> trade;
    trade.reset(new OTTrade(
        core,
        notaryID,
        instrumentDefinitionID,
        assetAcctId,
        nymID,
        currencyId,
        currencyAcctId));

    return trade;
}

std::unique_ptr<OTTransactionType> Factory::Transaction(
    const api::Core& core,
    const String& strInput) const
{
    String strContract, strFirstLine;  // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        std::unique_ptr<OTTransactionType> pContract;

        if (strFirstLine.Contains(
                "-----BEGIN SIGNED TRANSACTION-----"))  // this string is 34
                                                        // chars long.
        {
            pContract.reset(new OTTransaction(core));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine.Contains(
                       "-----BEGIN SIGNED TRANSACTION ITEM-----"))  // this
                                                                    // string is
                                                                    // 39 chars
                                                                    // long.
        {
            pContract.reset(new opentxs::Item(core));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine.Contains(
                       "-----BEGIN SIGNED LEDGER-----"))  // this string is 29
                                                          // chars long.
        {
            pContract.reset(new opentxs::Ledger(core));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine.Contains(
                       "-----BEGIN SIGNED ACCOUNT-----"))  // this string is 30
                                                           // chars long.
        {
            OT_FAIL;
        }

        // The string didn't match any of the options in the factory.
        //

        //        const char* szFunc = "OTTransactionType::TransactionFactory";
        // The string didn't match any of the options in the factory.
        if (nullptr == pContract) {
            otOut  //<< szFunc
                << ": Object type not yet supported by class factory: "
                << strFirstLine << "\n";
            return nullptr;
        }

        // This causes pItem to load ASSUMING that the PurportedAcctID and
        // PurportedNotaryID are correct.
        // The object is still expected to be internally consistent with its
        // sub-items, regarding those IDs,
        // but the big difference is that it will SET the Real Acct and Real
        // Notary IDs based on the purported
        // values. This way you can load a transaction without knowing the
        // account in advance.
        //
        pContract->SetLoadInsecure();

        // Does the contract successfully load from the string passed in?
        if (pContract->LoadContractFromString(strContract)) {
            // NOTE: this already happens in OTTransaction::ProcessXMLNode and
            // OTLedger::ProcessXMLNode.
            // Specifically, it happens when m_bLoadSecurely is set to false.
            //
            //          pContract->SetRealNotaryID(pItem->GetPurportedNotaryID());
            //          pContract->SetRealAccountID(pItem->GetPurportedAccountID());

            return pContract;
        } else {
            otOut  //<< szFunc
                << ": Failed loading contract from string (first line): "
                << strFirstLine << "\n";
        }
    }

    return nullptr;
}

std::unique_ptr<OTTransaction> Factory::Transaction(
    const api::Core& core,
    const opentxs::Ledger& theOwner) const
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(core, theOwner));

    return transaction;
}

std::unique_ptr<OTTransaction> Factory::Transaction(
    const api::Core& core,
    const Identifier& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    originType theOriginType) const
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        core, theNymID, theAccountID, theNotaryID, theOriginType));

    return transaction;
}

std::unique_ptr<OTTransaction> Factory::Transaction(
    const api::Core& core,
    const Identifier& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    std::int64_t lTransactionNum,
    originType theOriginType) const
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        core,
        theNymID,
        theAccountID,
        theNotaryID,
        lTransactionNum,
        theOriginType));

    return transaction;
}
// THIS factory only used when loading an abbreviated box receipt
// (inbox, nymbox, or outbox receipt).
// The full receipt is loaded only after the abbreviated ones are loaded,
// and verified against them.
std::unique_ptr<OTTransaction> Factory::Transaction(
    const api::Core& core,
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
    NumList* pNumList) const
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        core,
        theNymID,
        theAccountID,
        theNotaryID,
        lNumberOfOrigin,
        theOriginType,
        lTransactionNum,
        lInRefTo,
        lInRefDisplay,
        the_DATE_SIGNED,
        theType,
        strHash,
        lAdjustment,
        lDisplayValue,
        lClosingNum,
        lRequestNum,
        bReplyTransSuccess,
        pNumList));

    return transaction;
}

std::unique_ptr<OTTransaction> Factory::Transaction(
    const api::Core& core,
    const opentxs::Ledger& theOwner,
    transactionType theType,
    originType theOriginType /*=originType::not_applicable*/,
    std::int64_t lTransactionNum /*=0*/) const
{
    auto pTransaction = Transaction(
        core,
        theOwner.GetNymID(),
        theOwner.GetPurportedAccountID(),
        theOwner.GetPurportedNotaryID(),
        theType,
        theOriginType,
        lTransactionNum);
    if (false != bool(pTransaction)) pTransaction->SetParent(theOwner);

    return pTransaction;
}

std::unique_ptr<OTTransaction> Factory::Transaction(
    const api::Core& core,
    const Identifier& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    transactionType theType,
    originType theOriginType /*=originType::not_applicable*/,
    std::int64_t lTransactionNum /*=0*/) const
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        core,
        theNymID,
        theAccountID,
        theNotaryID,
        lTransactionNum,
        theOriginType));
    OT_ASSERT(false != bool(transaction));

    transaction->m_Type = theType;

    // Since we're actually generating this transaction, then we can go ahead
    // and set the purported account and server IDs (we have already set the
    // real ones in the constructor). Now both sets are fill with matching data.
    // No need to security check the IDs since we are creating this transaction
    // versus loading and inspecting it.
    transaction->SetPurportedAccountID(theAccountID);
    transaction->SetPurportedNotaryID(theNotaryID);

    return transaction;
}

}  // namespace opentxs::api::implementation
