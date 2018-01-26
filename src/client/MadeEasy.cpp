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

#include "opentxs/stdafx.hpp"

#include "opentxs/client/MadeEasy.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/client/OTAPI_Func.hpp"
#include "opentxs/client/SwigWrap.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#include <algorithm>
#include <iostream>

#define OT_METHOD "opentxs::MadeEasy::"

namespace opentxs
{
MadeEasy::MadeEasy(
    std::recursive_mutex& lock,
    OTAPI_Exec& exec,
    OT_API& otapi,
    api::client::Wallet& wallet)
    : lock_(lock)
    , exec_(exec)
    , otapi_(otapi)
    , wallet_(wallet)
{
}

// RETRIEVE NYM INTERMEDIARY FILES
// Returns:
//  True if I have enough numbers, or if there was success getting more
// transaction numbers.
//  False if I didn't have enough numbers, tried to get more, and failed
// somehow.
//
std::int32_t MadeEasy::retrieve_nym(
    const std::string& strNotaryID,
    const std::string& strMyNymID,
    bool& bWasMsgSent,
    bool bForceDownload) const
{
    rLock lock(lock_);
    auto context = wallet_.mutable_ServerContext(
        Identifier(strMyNymID), Identifier(strNotaryID));
    Utility MsgUtil(context.It(), otapi_);

    if (0 >= context.It().UpdateRequestNumber()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed calling getRequestNumber" << std::endl;

        return -1;
    } else  // If it returns 1, we know for sure that the request number is in
            // sync.
    {
        otInfo << "SUCCESS syncronizing the request number." << std::endl;
    }

    std::int32_t nGetAndProcessNymbox = MsgUtil.getAndProcessNymbox_4(
        strNotaryID, strMyNymID, bWasMsgSent, bForceDownload);

    return nGetAndProcessNymbox;
}

// CHECK USER (download a public key)
std::string MadeEasy::check_nym(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& TARGET_nymID) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        CHECK_NYM,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        TARGET_nymID);

    return theRequest.Run();
}

//  ISSUE ASSET TYPE
std::string MadeEasy::issue_asset_type(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& THE_CONTRACT) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        ISSUE_ASSET_TYPE,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        THE_CONTRACT);

    return theRequest.Run();
}

//  ISSUE BASKET CURRENCY
std::string MadeEasy::issue_basket_currency(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& THE_BASKET) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        ISSUE_BASKET,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        THE_BASKET);

    return theRequest.Run();
}

//  EXCHANGE BASKET CURRENCY
std::string MadeEasy::exchange_basket_currency(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ASSET_TYPE,
    const std::string& THE_BASKET,
    const std::string& ACCT_ID,
    bool IN_OR_OUT) const
{
    rLock lock(lock_);
    std::int32_t nTransNumsNeeded =
        (SwigWrap::Basket_GetMemberCount(THE_BASKET) + 1);
    OTAPI_Func theRequest(
        EXCHANGE_BASKET,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        ASSET_TYPE,
        THE_BASKET,
        ACCT_ID,
        IN_OR_OUT,
        nTransNumsNeeded);

    return theRequest.Run();
}

//  RETRIEVE CONTRACT
std::string MadeEasy::retrieve_contract(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& CONTRACT_ID) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        GET_CONTRACT,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        CONTRACT_ID);

    return theRequest.Run();
}

//  LOAD OR RETRIEVE CONTRACT
std::string MadeEasy::load_or_retrieve_contract(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& CONTRACT_ID) const
{
    rLock lock(lock_);

    std::string strContract = SwigWrap::GetAssetType_Contract(CONTRACT_ID);

    if (!VerifyStringVal(strContract)) {
        std::string strResponse =
            retrieve_contract(notaryID, nymID, CONTRACT_ID);

        if (1 == VerifyMessageSuccess(strResponse)) {
            strContract = SwigWrap::GetAssetType_Contract(CONTRACT_ID);
        }
    }

    return strContract;  // might be null.
}

//  CREATE ASSET ACCOUNT
std::string MadeEasy::create_asset_acct(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        CREATE_ASSET_ACCT,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        INSTRUMENT_DEFINITION_ID);

    return theRequest.Run();
}

//  UNREGISTER ASSET ACCOUNT
std::string MadeEasy::unregister_account(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCOUNT_ID) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        DELETE_ASSET_ACCT,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        ACCOUNT_ID);

    return theRequest.Run();
}

//  UNREGISTER NYM FROM SERVER
std::string MadeEasy::unregister_nym(
    const std::string& notaryID,
    const std::string& nymID) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        DELETE_NYM,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_);

    return theRequest.Run();
}

std::string MadeEasy::stat_asset_account(const std::string& ACCOUNT_ID) const
{
    rLock lock(lock_);

    std::string strNymID = SwigWrap::GetAccountWallet_NymID(ACCOUNT_ID);
    if (!VerifyStringVal(strNymID)) {
        otOut << "\nstat_asset_account: Cannot find account wallet for: "
              << ACCOUNT_ID << "\n";
        return "";
    }

    std::string strInstrumentDefinitionID =
        SwigWrap::GetAccountWallet_InstrumentDefinitionID(ACCOUNT_ID);
    if (!VerifyStringVal(strInstrumentDefinitionID)) {
        otOut << "\nstat_asset_account: Cannot cannot determine instrument "
                 "definition for: "
              << ACCOUNT_ID << "\n";
        return "";
    }

    std::string strName = SwigWrap::GetAccountWallet_Name(ACCOUNT_ID);
    std::string strNotaryID = SwigWrap::GetAccountWallet_NotaryID(ACCOUNT_ID);
    std::int64_t lBalance = SwigWrap::GetAccountWallet_Balance(ACCOUNT_ID);
    std::string strAssetTypeName =
        SwigWrap::GetAssetType_Name(strInstrumentDefinitionID);
    std::string strNymName = SwigWrap::GetNym_Name(strNymID);
    std::string strServerName = SwigWrap::GetServer_Name(strNotaryID);

    return "   Balance: " +
           SwigWrap::FormatAmount(strInstrumentDefinitionID, lBalance) +
           "   (" + strName + ")\nAccount ID: " + ACCOUNT_ID + " ( " + strName +
           " )\nAsset Type: " + strInstrumentDefinitionID + " ( " +
           strAssetTypeName + " )\nOwner Nym : " + strNymID + " ( " +
           strNymName + " )\nServer    : " + strNotaryID + " ( " +
           strServerName + " )";
}

// DOWNLOAD ACCOUNT FILES  (account balance, inbox, outbox, etc)
// returns true/false
bool MadeEasy::retrieve_account(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCOUNT_ID,
    bool bForceDownload) const  // bForceDownload=false
{
    rLock lock(lock_);
    auto context =
        wallet_.mutable_ServerContext(Identifier(nymID), Identifier(notaryID));
    Utility MsgUtil(context.It(), otapi_);

    bool bResponse = MsgUtil.getIntermediaryFiles(
        notaryID, nymID, ACCOUNT_ID, bForceDownload);

    return bResponse;
}

// SEND TRANSFER  -- TRANSACTION
std::string MadeEasy::send_transfer(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_FROM,
    const std::string& ACCT_TO,
    const std::int64_t AMOUNT,
    const std::string& NOTE,
    TransactionNumber& number) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        SEND_TRANSFER,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        ACCT_FROM,
        ACCT_TO,
        AMOUNT,
        NOTE);
    const auto output = theRequest.Run();
    number = theRequest.GetTransactionNumber();

    return output;
}

// PROCESS INBOX  -- TRANSACTION
std::string MadeEasy::process_inbox(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCOUNT_ID,
    const std::string& RESPONSE_LEDGER) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        PROCESS_INBOX,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        ACCOUNT_ID,
        RESPONSE_LEDGER);

    return theRequest.Run();
}

// load_public_key():
//
// Load a public key from local storage, and return it (or null).
//
// TODO: Need to fix ugly error messages by passing a bChecking in here
// so the calling function can try to load the pubkey just to see if it's there,
// without causing ugly error logs when there's no error.
std::string MadeEasy::load_public_encryption_key(const std::string& nymID) const
{
    rLock lock(lock_);

    otOut << "\nload_public_encryption_key: Trying to load public "
             "key, assuming Nym isn't in the local wallet...\n";

    std::string strPubkey = SwigWrap::LoadPubkey_Encryption(
        nymID);  // This version is for "other people";

    if (!VerifyStringVal(strPubkey)) {
        otOut << "\nload_public_encryption_key: Didn't find the Nym (" << nymID
              << ") as an 'other' user, so next, checking to see if there's "
                 "a pubkey available for one of the local private Nyms...\n";
        strPubkey = SwigWrap::LoadUserPubkey_Encryption(
            nymID);  // This version is for "the user sitting at the machine.";

        if (!VerifyStringVal(strPubkey)) {
            otOut << "\nload_public_encryption_key: Didn't find "
                     "him as a local Nym either... returning nullptr.\n";
        }
    }
    return strPubkey;  // might be null.;
}

// load_public_key():
//
// Load a public key from local storage, and return it (or null).
std::string MadeEasy::load_public_signing_key(const std::string& nymID) const
{
    rLock lock(lock_);

    std::string strPubkey = SwigWrap::LoadPubkey_Signing(
        nymID);  // This version is for "other people";

    if (!VerifyStringVal(strPubkey)) {
        strPubkey = SwigWrap::LoadUserPubkey_Signing(
            nymID);  // This version is for "the user sitting at the machine.";
    }
    return strPubkey;  // might be null.;
}

// load_or_retrieve_pubkey()
//
// Load TARGET_nymID from local storage.
// If not there, then retrieve TARGET_nymID from server,
// using nymID to send check_nym request. Then re-load
// and return. (Might still return null.)
std::string MadeEasy::load_or_retrieve_encrypt_key(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& TARGET_nymID) const
{
    rLock lock(lock_);

    std::string strPubkey = load_public_encryption_key(TARGET_nymID);

    if (!VerifyStringVal(strPubkey)) {
        std::string strResponse = check_nym(notaryID, nymID, TARGET_nymID);

        if (1 == VerifyMessageSuccess(strResponse)) {
            strPubkey = load_public_encryption_key(TARGET_nymID);
        }
    }
    return strPubkey;  // might be null.
}

// SEND USER MESSAGE  (requires recipient public key)
std::string MadeEasy::send_user_msg_pubkey(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& RECIPIENT_nymID,
    const std::string& RECIPIENT_PUBKEY,
    const std::string& THE_MESSAGE) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        SEND_USER_MESSAGE,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        RECIPIENT_nymID,
        RECIPIENT_PUBKEY,
        THE_MESSAGE);

    return theRequest.Run();
}

// SEND USER INSTRUMENT  (requires recipient public key)
std::string MadeEasy::send_user_pmnt_pubkey(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& RECIPIENT_nymID,
    const std::string& RECIPIENT_PUBKEY,
    const std::string& THE_INSTRUMENT) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        SEND_USER_INSTRUMENT,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        RECIPIENT_nymID,
        RECIPIENT_PUBKEY,
        THE_INSTRUMENT);

    return theRequest.Run();
}

#if OT_CASH
// SEND USER CASH  (requires recipient public key)
std::string MadeEasy::send_user_cash_pubkey(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& RECIPIENT_nymID,
    const std::string& RECIPIENT_PUBKEY,
    const std::string& THE_INSTRUMENT,
    const std::string& INSTRUMENT_FOR_SENDER) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        SEND_USER_INSTRUMENT,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        RECIPIENT_nymID,
        RECIPIENT_PUBKEY,
        THE_INSTRUMENT,
        INSTRUMENT_FOR_SENDER);

    return theRequest.Run();
}
#endif  // OT_CASH

// SEND USER MESSAGE  (only requires recipient's ID, and retrieves pubkey
// automatically)
std::string MadeEasy::send_user_msg(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& RECIPIENT_nymID,
    const std::string& THE_MESSAGE) const
{
    rLock lock(lock_);

    std::string strRecipientPubkey =
        load_or_retrieve_encrypt_key(notaryID, nymID, RECIPIENT_nymID);

    if (!VerifyStringVal(strRecipientPubkey)) {
        otOut << "OT_ME_send_user_msg: Unable to load or retrieve "
                 "public encryption key for recipient: "
              << RECIPIENT_nymID << "\n";
        return strRecipientPubkey;  // basically this means "return null".
    }

    return send_user_msg_pubkey(
        notaryID, nymID, RECIPIENT_nymID, strRecipientPubkey, THE_MESSAGE);
}

#if OT_CASH
// DOWNLOAD PUBLIC MINT
std::string MadeEasy::retrieve_mint(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        GET_MINT,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        INSTRUMENT_DEFINITION_ID);

    return theRequest.Run();
}

// LOAD MINT (from local storage)
//
// To load a mint withOUT retrieving it from server, call:
//
// var strMint = SwigWrap::LoadMint(notaryID, INSTRUMENT_DEFINITION_ID);
// It returns the mint, or null.

// LOAD MINT (from local storage).
// Also, if necessary, RETRIEVE it from the server first.
//
// Returns the mint, or null.
std::string MadeEasy::load_or_retrieve_mint(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    rLock lock(lock_);

    std::string response = check_nym(notaryID, nymID, nymID);
    if (1 != VerifyMessageSuccess(response)) {
        otOut << "OT_ME_load_or_retrieve_mint: Cannot verify nym for IDs: \n";
        otOut << "  Notary ID: " << notaryID << "\n";
        otOut << "     Nym ID: " << nymID << "\n";
        otOut << "   Instrument Definition Id: " << INSTRUMENT_DEFINITION_ID
              << "\n";
        return "";
    }

    // HERE, WE MAKE SURE WE HAVE THE PROPER MINT...
    //
    // Download the public mintfile if it's not there, or if it's expired.
    // Also load it up into memory as a std::string (just to make sure it
    // works.)

    // expired or missing.
    if (!SwigWrap::Mint_IsStillGood(notaryID, INSTRUMENT_DEFINITION_ID)) {
        otWarn << "OT_ME_load_or_retrieve_mint: Mint file is "
                  "missing or expired. Downloading from "
                  "server...\n";

        response = retrieve_mint(notaryID, nymID, INSTRUMENT_DEFINITION_ID);

        if (1 != VerifyMessageSuccess(response)) {
            otOut << "OT_ME_load_or_retrieve_mint: Unable to "
                     "retrieve mint for IDs: \n";
            otOut << "  Notary ID: " << notaryID << "\n";
            otOut << "     Nym ID: " << nymID << "\n";
            otOut << "   Instrument Definition Id: " << INSTRUMENT_DEFINITION_ID
                  << "\n";
            return "";
        }

        if (!SwigWrap::Mint_IsStillGood(notaryID, INSTRUMENT_DEFINITION_ID)) {
            otOut << "OT_ME_load_or_retrieve_mint: Retrieved "
                     "mint, but still 'not good' for IDs: \n";
            otOut << "  Notary ID: " << notaryID << "\n";
            otOut << "     Nym ID: " << nymID << "\n";
            otOut << "   Instrument Definition Id: " << INSTRUMENT_DEFINITION_ID
                  << "\n";
            return "";
        }
    }
    // else // current mint IS available already on local storage (and not
    // expired.)

    // By this point, the mint is definitely good, whether we had to download it
    // or not.
    // It's here, and it's NOT expired. (Or we would have returned already.)

    std::string strMint =
        SwigWrap::LoadMint(notaryID, INSTRUMENT_DEFINITION_ID);
    if (!VerifyStringVal(strMint)) {
        otOut << "OT_ME_load_or_retrieve_mint: Unable to load mint for IDs: \n";
        otOut << "  Notary ID: " << notaryID << "\n";
        otOut << "     Nym ID: " << nymID << "\n";
        otOut << "   Instrument Definition Id: " << INSTRUMENT_DEFINITION_ID
              << "\n";
    }

    return strMint;
}
#endif  // OT_CASH

// DEPOSIT PAYMENT PLAN  -- TRANSACTION
std::string MadeEasy::deposit_payment_plan(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& THE_PAYMENT_PLAN) const
{
    rLock lock(lock_);
    // NOTE: We have to include the account ID as well. Even though the API call
    // itself doesn't need it (it retrieves it from the plan itself, as we are
    // about to do here) we still have to provide the accountID for OTAPI_Func,
    // which uses it to grab the intermediary files, as part of its automated
    // sync duties. (FYI.)
    const auto strSenderAcctID =
        SwigWrap::Instrmnt_GetSenderAcctID(THE_PAYMENT_PLAN);
    OTAPI_Func theRequest(
        DEPOSIT_PAYMENT_PLAN,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        strSenderAcctID,
        THE_PAYMENT_PLAN);

    return theRequest.Run();
}

#if OT_CASH
// Imports a purse into the wallet.
// NOTE:   UNUSED currently.
bool MadeEasy::importCashPurse(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& instrumentDefinitionID,
    std::string& userInput,
    bool isPurse) const
{
    rLock lock(lock_);

    //  otOut << "OT_ME_importCashPurse, notaryID:" << notaryID << "
    // nymID:" << nymID << " instrumentDefinitionID:" <<
    // instrumentDefinitionID);
    //  otOut << "OT_ME_importCashPurse, userInput purse:" <<
    // userInput <<);

    if (!isPurse)  // it's not a purse. Must be a
                   // token, so let's create a purse
                   // for it.
    {
        //      otOut << "OT_ME_importCashPurse, isPurse:" +
        // isPurse)

        std::string purse = SwigWrap::CreatePurse(
            notaryID,
            instrumentDefinitionID,
            nymID,
            nymID);  // nymID, nymID == owner, signer;

        if (!VerifyStringVal(purse)) {
            otOut << "OT_ME_importCashPurse: Error: "
                     "OT_API_CreatePurse returned null\n";
            return false;
        }
        //      otOut << "OT_ME_importCashPurse, OT_API_CreatePurse
        // return :" + purse);

        std::string newPurse = SwigWrap::Purse_Push(
            notaryID, instrumentDefinitionID, nymID, nymID, purse, userInput);
        if (!VerifyStringVal(newPurse)) {
            otOut << "OT_ME_importCashPurse: Error: "
                     "OT_API_Purse_Push returned null\n";
            return false;
        }
        //      otOut << "OT_ME_importCashPurse, OT_API_Purse_Push
        // returned :" + newPurse);
        userInput = newPurse;
    }
    //  otOut << "OT_ME_importCashPurse, Before calling
    // OT_API_Wallet_ImportPurse, final purse:" + userInput);
    //  otOut << "OT_ME_importCashPurse just before api ,
    // notaryID:" + notaryID + " nymID:" + nymID + " instrumentDefinitionID:" +
    // instrumentDefinitionID);

    // Here we have either a purse that was passed in, or a purse that we
    // created so
    // we could add the token that was passed in. Either way, we have a purse
    // now, so
    // let's import it into the wallet.
    //
    return 1 == SwigWrap::Wallet_ImportPurse(
                    notaryID, instrumentDefinitionID, nymID, userInput);
}

// processCashPurse pops the selected tokens off of oldPurse, changes their
// owner to recipientNymID,
// and pushes them onto newPurse, owned by recipientNymID. Meanwhile any
// unselected tokens are pushed
// onto newPurseUnselectedTokens, owned by nymID, which is saved to local
// storage (overwriting...)
// newPurse is returned from this function.
//
// Thus, for example, if there's a problem depositing/exporting/exchanging
// newPurse, then it needs
// to be merged back into the old purse again, which is otherwise still missing
// those tokens. (We
// should keep a copy of them in the payments outbox or something like that, in
// the meantime.)
//
// What is newPurseForSender for?
// Sometimes when processCashPurse is called, a Nym is just removing tokens from
// his purse so he can
// deposit them into his own account. Meaning the "sender" Nym (who owns the
// purse) is the same as the
// "recipient" Nym (who owns the account.) In which case, newPurse (for
// recipient) and newPurseForSender
// will contain the exact same thing. But in the case where they are different
// Nyms (like if I am
// exporting these tokens from my purse in order to give them to someone else)
// then I will want a copy
// of the exported purse for the recipient, and a copy of the same exported
// purse for the sender.
// processCashPurse returns both of those for me. The reason is because
// otherwise, if I just encrypted
// some of my cash tokens to some external Nym, then only THAT Nym's private key
// will ever open them up
// again. Even if I have them in my outpayments box, I can never recover them
// because I can never again
// decrypt them. Therefore I make a special copy of the outgoing purse which is
// encrypted to my own key,
// and put that into my outpayments box instead of the one I sent to the
// recipient (or including both
// of them.) That way the cash can still be recovered if necessary, from my
// outpayments box, and re-imported
// back into my cash purse again.
//
bool MadeEasy::processCashPurse(
    std::string& newPurse,
    std::string& newPurseForSender,
    const std::string& notaryID,
    const std::string& instrumentDefinitionID,
    const std::string& nymID,
    std::string& oldPurse,
    const std::vector<std::string>& selectedTokens,
    const std::string& recipientNymID,
    bool bPWProtectOldPurse,
    bool bPWProtectNewPurse) const
{
    rLock lock(lock_);

    // By this point, we know that "selected tokens" has a size of 0, or MORE
    // THAN ONE. (But NOT 1 exactly.)
    // (At least, if this function was called by exportCashPurse.)
    std::string strLocation = "OT_ME_processCashPurse";

    // This block handles cases where NO TOKENS ARE SELECTED.
    //
    // (Meaning: "PROCESS" THEM ALL.)
    //
    if (selectedTokens.size() < 1) {
        // newPurse is created, OWNED BY RECIPIENT.
        //
        newPurse =
            (bPWProtectNewPurse ? SwigWrap::CreatePurse_Passphrase(
                                      notaryID, instrumentDefinitionID, nymID)
                                : SwigWrap::CreatePurse(
                                      notaryID,
                                      instrumentDefinitionID,
                                      recipientNymID,
                                      nymID));  // recipientNymID is owner,
                                                // nymID is signer;

        if (!VerifyStringVal(newPurse)) {
            otOut << strLocation << ": "
                  << (bPWProtectNewPurse ? "OT_API_CreatePurse_Passphrase"
                                         : "OT_API_CreatePurse")
                  << " returned null\n";
            return false;
        }

        // We'll create an extra copy of the newPurse, which is encrypted to the
        // sender (instead of the recipient or
        // some passphrase.) We'll call it newPurseForSender. This way the
        // sender can later have the option to recover
        // the cash from his outbox.
        //
        newPurseForSender = SwigWrap::CreatePurse(
            notaryID,
            instrumentDefinitionID,
            nymID,
            nymID);  // nymID is owner, nymID is signer;

        if (!VerifyStringVal(newPurseForSender)) {
            otOut << strLocation
                  << ": Failure: OT_API_CreatePurse returned null\n";
            return false;
        }

        // Iterate through the OLD PURSE. (as tempOldPurse.)
        //
        std::int32_t count =
            SwigWrap::Purse_Count(notaryID, instrumentDefinitionID, oldPurse);
        std::string tempOldPurse = oldPurse;

        for (std::int32_t i = 0; i < count; ++i) {
            // Peek into TOKEN, from the top token on the stack. (And it's STILL
            // on top after this call.)
            //
            std::string token = SwigWrap::Purse_Peek(
                notaryID, instrumentDefinitionID, nymID, tempOldPurse);

            // Now pop the token off of tempOldPurse (our iterator for the old
            // purse).
            // Store updated copy of purse (sans token) into "str1".
            //
            std::string str1 = SwigWrap::Purse_Pop(
                notaryID, instrumentDefinitionID, nymID, tempOldPurse);

            if (!VerifyStringVal(token) || !VerifyStringVal(str1)) {
                otOut << strLocation
                      << ": OT_API_Purse_Peek or OT_API_Purse_Pop "
                         "returned null... SHOULD NEVER HAPPEN. "
                         "Returning null.\n";
                return false;
            }

            // Since pop succeeded, copy the output to tempOldPurse
            // (for next iteration, in case any continues happen below.)
            // Now tempOldPurse contains what it did before, MINUS ONE TOKEN.
            // (The exported one.)
            //
            tempOldPurse = str1;

            std::string strSender = bPWProtectOldPurse ? oldPurse : nymID;
            std::string strRecipient =
                bPWProtectNewPurse ? newPurse : recipientNymID;

            std::string strSenderAsRecipient =
                nymID;  // Used as the "owner" of newPurseForSender. (So the
                        // sender can recover his sent coins that got encrypted
                        // to someone else's key.);

            // Change the OWNER on token, from NymID to RECIPIENT.
            // (In this block, we change ALL the tokens in the purse.)
            //
            std::string exportedToken = SwigWrap::Token_ChangeOwner(
                notaryID,
                instrumentDefinitionID,
                token,
                nymID,          // signer ID
                strSender,      // old owner
                strRecipient);  // new owner
            // If change failed, then continue.
            //
            if (!VerifyStringVal(exportedToken)) {
                otOut << strLocation << ": 1, OT_API_Token_ChangeOwner "
                                        "returned null...(should never "
                                        "happen) Returning null.\n";
                return false;
            }

            // SAVE A COPY FOR THE SENDER...
            //
            std::string retainedToken = SwigWrap::Token_ChangeOwner(
                notaryID,
                instrumentDefinitionID,
                token,
                nymID,                  // signer ID
                strSender,              // old owner
                strSenderAsRecipient);  // new owner
            // If change failed, then continue.
            //
            if (!VerifyStringVal(retainedToken)) {
                otOut << strLocation << ":  2, OT_API_Token_ChangeOwner "
                                        "returned null...(should never "
                                        "happen) Returning null.\n";
                return false;
            }

            //          strSender    = bPWProtectOldPurse ? "" : nymID // unused
            // here. not needed.
            strRecipient = bPWProtectNewPurse ? "" : recipientNymID;

            // PUSH the EXPORTED TOKEN (new owner) into the new purse (again,
            // recipient/newPurse is new owner) and save results in
            // "strPushedForRecipient".
            // Results are, FYI, newPurse+exportedToken.
            //
            std::string strPushedForRecipient = SwigWrap::Purse_Push(
                notaryID,
                instrumentDefinitionID,
                nymID,         // server, asset, signer
                strRecipient,  // owner is either nullptr (for
                               // password-protected
                               // purse) or recipientNymID
                newPurse,
                exportedToken);  // purse, token

            // If push failed, then continue.
            if (!VerifyStringVal(strPushedForRecipient)) {
                otOut << strLocation
                      << ":  OT_API_Purse_Push 1 returned null... "
                         "(should never happen) Returning null.\n";
                return false;
            }

            // PUSH the RETAINED TOKEN (copy for original owner) into the
            // newPurseForSender and save results in "strPushedForRetention".
            // Results are, FYI, newPurseForSender+retainedToken.
            //
            std::string strPushedForRetention = SwigWrap::Purse_Push(
                notaryID,
                instrumentDefinitionID,
                nymID,                 // server, asset, signer
                strSenderAsRecipient,  // This version of the purse is the
                // outgoing copy (for the SENDER's notes).
                // Thus strSenderAsRecipient.
                newPurseForSender,
                retainedToken);  // purse, token

            // If push failed, then continue.
            if (!VerifyStringVal(strPushedForRetention)) {
                otOut << strLocation
                      << ":  OT_API_Purse_Push 2 returned null... "
                         "(should never happen) Returning null.\n";
                return false;
            }

            // Since push succeeded, copy "strPushedForRecipient" (containing
            // newPurse         +exportedToken) into newPurse.
            // Since push succeeded, copy "strPushedForRetention" (containing
            // newPurseForSender+retainedToken) into newPurseForSender.
            //
            newPurse = strPushedForRecipient;
            newPurseForSender = strPushedForRetention;
        }  // for

        // Save tempOldPurse to local storage. (For OLD Owner.)
        // By now, all of the tokens have been popped off of this purse, so it
        // is EMPTY.
        // We're now saving the empty purse, since the user exported all of the
        // tokens.
        //
        // THERE MAYBE SHOULD BE AN EXTRA MODAL HERE, that says,
        // "Moneychanger will now save your purse, EMPTY, back to local storage.
        // Are you sure you want to do this?"
        //

        if (!bPWProtectOldPurse)  // If old purse is NOT password-protected
                                  // (that
                                  // is, it's encrypted to a Nym.)
        {
            if (!SwigWrap::SavePurse(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,
                    tempOldPurse))  // if FAILURE.
            {
                // No error message if saving fails??
                // No modal?
                //
                // FT: adding log.
                otOut << strLocation << ": OT_API_SavePurse "
                                        "FAILED. SHOULD NEVER HAPPEN!!!!!!\n";
                return false;
            }
        } else  // old purse IS password protected. (So return its updated
                // version.)
        {
            oldPurse =
                tempOldPurse;  // We never cared about this with Nym-owned
                               // old purse, since it saves to storage
                               // anyway, in the above block. But now in
                               // the case of password-protected purses,
                               // we set the oldPurse to contain the new
                               // version of itself (containing the tokens
                               // that had been left unselected) so the
                               // caller can do what he wills with it.;
        }
    }

    // Else, SPECIFIC TOKENS were selected, so process those only...
    //
    else {
        //      otOut << "Tokens in Cash Purse being processed");

        // newPurseSelectedTokens is created (CORRECTLY) with recipientNymID as
        // owner. (Or with a symmetric key / passphrase.)
        // newPurseUnSelectedTokens is created (CORRECTLY) with NymID as owner.
        // (Unselected tokens aren't being exported...)
        //
        std::string newPurseUnSelectedTokens = SwigWrap::Purse_Empty(
            notaryID,
            instrumentDefinitionID,
            nymID,
            oldPurse);  // Creates an empty copy of oldPurse.;
        std::string newPurseSelectedTokens =
            (bPWProtectNewPurse ? SwigWrap::CreatePurse_Passphrase(
                                      notaryID, instrumentDefinitionID, nymID)
                                : SwigWrap::CreatePurse(
                                      notaryID,
                                      instrumentDefinitionID,
                                      recipientNymID,
                                      nymID));  // recipientNymID = owner,
                                                // nymID = signer;
        std::string newPurseSelectedForSender = SwigWrap::CreatePurse(
            notaryID,
            instrumentDefinitionID,
            nymID,
            nymID);  // nymID = owner, nymID = signer. This is a copy of
                     // newPurseSelectedTokens that's encrypted to the SENDER
                     // (for putting in his outpayments box, so he can still
                     // decrypt if necessary.);

        if (!VerifyStringVal(newPurseSelectedForSender)) {
            otOut << strLocation << ":  OT_API_CreatePurse returned null\n";
            return false;
        }
        if (!VerifyStringVal(newPurseSelectedTokens)) {
            otOut << strLocation
                  << ":  OT_API_CreatePurse or "
                     "OT_API_CreatePurse_Passphrase returned null\n";
            return false;
        }
        if (!VerifyStringVal((newPurseUnSelectedTokens))) {
            otOut << strLocation << ":  OT_API_Purse_Empty returned null\n";
            return false;
        }

        // Iterate through oldPurse, using tempOldPurse as iterator.
        //
        std::int32_t count =
            SwigWrap::Purse_Count(notaryID, instrumentDefinitionID, oldPurse);
        std::string tempOldPurse = oldPurse;

        for (std::int32_t i = 0; i < count; ++i) {
            // Peek at the token on top of the stack.
            // (Without removing it.)
            //
            std::string token = SwigWrap::Purse_Peek(
                notaryID, instrumentDefinitionID, nymID, tempOldPurse);

            // Remove the top token from the stack, and return the updated stack
            // in "str1".
            //
            std::string str1 = SwigWrap::Purse_Pop(
                notaryID, instrumentDefinitionID, nymID, tempOldPurse);

            if (!VerifyStringVal(str1) || !VerifyStringVal(token)) {
                otOut << strLocation
                      << ":  OT_API_Purse_Peek or "
                         "OT_API_Purse_Pop returned null... returning Null. "
                         "(SHOULD NEVER HAPPEN.)\n";
                return false;
            }

            // Putting updated purse into iterator, so any subsequent continues
            // will work properly.
            //
            tempOldPurse = str1;

            // Grab the TokenID for that token. (Token still has OLD OWNER.)
            //
            std::string tokenID =
                SwigWrap::Token_GetID(notaryID, instrumentDefinitionID, token);

            if (!VerifyStringVal(tokenID)) {
                otOut << strLocation
                      << ":  OT_API_Token_GetID returned null... "
                         "SHOULD NEVER HAPPEN. Returning now.\n";
                return false;
            }

            // At this point, we check TokenID (identifying the current token)
            // to see if it's on the SELECTED LIST.
            //
            if (find(selectedTokens.begin(), selectedTokens.end(), tokenID) !=
                selectedTokens.end())  // We ARE exporting
                                       // this token. (Its
                                       // ID was on the
                                       // list.)
            {
                // CHANGE OWNER from NYM to RECIPIENT
                // "token" will now contain the EXPORTED TOKEN, with the NEW
                // OWNER.
                //
                std::string strSender = bPWProtectOldPurse ? oldPurse : nymID;
                std::string strRecipient = bPWProtectNewPurse
                                               ? newPurseSelectedTokens
                                               : recipientNymID;

                std::string strSenderAsRecipient =
                    nymID;  // Used as the "owner" of newPurseSelectedForSender.
                // (So the sender can recover his sent coins that got
                // encrypted to someone else's key.);

                std::string exportedToken = SwigWrap::Token_ChangeOwner(
                    notaryID,
                    instrumentDefinitionID,
                    token,          // server, asset, token,;
                    nymID,          // signer nym
                    strSender,      // old owner
                    strRecipient);  // new owner
                if (!VerifyStringVal(exportedToken)) {
                    otOut << strLocation << ": 1  OT_API_Token_ChangeOwner "
                                            "returned null... SHOULD NEVER "
                                            "HAPPEN. Returning now.\n";
                    return false;
                }

                std::string retainedToken = SwigWrap::Token_ChangeOwner(
                    notaryID,
                    instrumentDefinitionID,
                    token,                  // server, asset, token,;
                    nymID,                  // signer nym
                    strSender,              // old owner
                    strSenderAsRecipient);  // new owner
                if (!VerifyStringVal(retainedToken)) {
                    otOut << strLocation << ": 2  OT_API_Token_ChangeOwner "
                                            "returned null... SHOULD NEVER "
                                            "HAPPEN. Returning now.\n";
                    return false;
                }

                // Push exported version of token into new purse for recipient
                // (for selected tokens.)
                //
                //              strSender    = bPWProtectOldPurse ? "" : nymID
                // // unused here. Not needed.
                strRecipient = bPWProtectNewPurse ? "" : recipientNymID;

                std::string strPushedForRecipient = SwigWrap::Purse_Push(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,         // server, asset, signer;
                    strRecipient,  // owner is either nullptr (for
                    // password-protected purse) or recipientNymID
                    newPurseSelectedTokens,
                    exportedToken);  // purse, token
                if (!VerifyStringVal(strPushedForRecipient)) {
                    otOut << strLocation
                          << ":  OT_API_Purse_Push "
                             "newPurseSelectedTokens returned null... "
                             "SHOULD NEVER HAPPEN (returning.)\n";
                    return false;
                }

                // Done: push a copy of these into a purse for the original
                // owner as well, so he has his OWN copy
                // to save in his payments outbox (that HE can decrypt...) so if
                // the cash is lost, for example, he can still
                // recover it. If the recipient receives it and deposits it
                // correctly, the cash in your payment outbox is now
                // worthless and can be discarded, although its existence may be
                // valuable to you as a receipt.
                //
                std::string strPushedForRetention = SwigWrap::Purse_Push(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,  // server, asset, signer;
                    strSenderAsRecipient,
                    newPurseSelectedForSender,
                    retainedToken);  // purse, token
                if (!VerifyStringVal(strPushedForRetention)) {
                    otOut << strLocation
                          << ":  OT_API_Purse_Push "
                             "newPurseSelectedForSender returned null... "
                             "SHOULD NEVER HAPPEN (returning.)\n";
                    return false;
                }

                newPurseSelectedTokens = strPushedForRecipient;
                newPurseSelectedForSender = strPushedForRetention;

            } else  // The token, this iteration, is NOT being exported, but is
                    // remaining with the original owner.
            {
                std::string strSender = bPWProtectOldPurse ? "" : nymID;

                std::string str = SwigWrap::Purse_Push(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,      // server, asset, signer;
                    strSender,  // owner is either nullptr (for
                                // password-protected
                                // purse) or nymID
                    newPurseUnSelectedTokens,
                    token);  // purse, token
                if (!VerifyStringVal(str)) {
                    otOut << strLocation
                          << ": OT_API_Purse_Push "
                             "newPurseUnSelectedTokens returned null... "
                             "SHOULD NEVER HAPPEN. Returning false.\n";
                    return false;
                }

                newPurseUnSelectedTokens = str;
            }
        }  // for

        if (!bPWProtectOldPurse)  // If old purse is NOT password-protected
                                  // (that
                                  // is, it's encrypted to a Nym.)
        {
            if (!SwigWrap::SavePurse(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,
                    newPurseUnSelectedTokens))  // if FAILURE.
            {
                // No error message if saving fails??
                // No modal?
                //
                // FT: adding log.
                otOut << strLocation << ":  OT_API_SavePurse "
                                        "FAILED. SHOULD NEVER HAPPEN!!!!!!\n";
                return false;
            }
        } else  // old purse IS password protected. (So return its updated
                // version.)
        {
            oldPurse =
                newPurseUnSelectedTokens;  // We never cared about this with
            // Nym-owned old purse, since it saves
            // to storage anyway, in the above
            // block. But now in the case of
            // password-protected purses, we set
            // the oldPurse to contain the new
            // version of itself (containing the
            // tokens that had been left
            // unselected) so the caller can do
            // what he wills with it.;
        }

        // The SELECTED tokens (with Recipient as owner of purse AND tokens
        // within) are returned as the "newPurse".
        // The SELECTED tokens (with Sender as owner of purse AND tokens within)
        // are returned as "newPurseForSender".
        //
        newPurse = newPurseSelectedTokens;
        newPurseForSender = newPurseSelectedForSender;
    }

    return true;
}

// Input: server ID, instrumentDefinitionID, Nym of current owner, existing
// purse, list of
// selected tokens, Nym of Recipient, and bool bPasswordProtected.
// Returns: "new Purse"
//
std::string MadeEasy::exportCashPurse(
    const std::string& notaryID,
    const std::string& instrumentDefinitionID,
    const std::string& nymID,
    const std::string& oldPurse,
    const std::vector<std::string>& selectedTokens,
    std::string& recipientNymID,
    bool bPasswordProtected,
    std::string& strRetainedCopy) const
{
    rLock lock(lock_);

    //  otOut << "OT_ME_exportCashPurse starts, selectedTokens:" <<
    // selectedTokens << "\n";
    //  Utility.setObj(null);

    if (!bPasswordProtected) {
        // If no recipient, then recipient == Nym.
        //
        if (!VerifyStringVal(recipientNymID) || (recipientNymID.size() == 0)) {
            otOut << "OT_ME_exportCashPurse: recipientNym empty--using NymID "
                     "for recipient instead: "
                  << nymID << "\n";
            recipientNymID = nymID;
        }

        if (!(recipientNymID == nymID)) {
            // Even though we don't use this variable after this point,
            // we've still done something important: loaded and possibly
            // downloaded the recipient Nym, so that later in this function
            // we can reference that recipientNymID in other calls and we know
            // it will work.
            //
            std::string recipientPubKey = load_or_retrieve_encrypt_key(
                notaryID, nymID, recipientNymID);  // this function handles
            // partial IDs for recipient.;

            if (!VerifyStringVal(recipientPubKey)) {
                otOut << "OT_ME_exportCashPurse: recipientPubKey is null\n";
                return "";
            }
        }
    }

    // By this point, we have verified that we can load the public key for the
    // recipient.
    // (IF the exported purse isn't meant to be password-protected.)
    //
    std::string token = "";
    std::string exportedToken = "";
    std::string exportedPurse = "";

    // Next I create another "newPurse" by calling this function.
    //
    std::string newPurse = "";  // for recipient;
    std::string newPurseForSender = "";
    std::string copyOfOldPurse = oldPurse;
    bool bSuccessProcess = processCashPurse(
        newPurse,
        newPurseForSender,
        notaryID,
        instrumentDefinitionID,
        nymID,
        copyOfOldPurse,
        selectedTokens,
        recipientNymID,
        false,
        bPasswordProtected);

    if (bSuccessProcess) {
        strRetainedCopy = newPurseForSender;
    }

    // Whatever is returned from that function, I return here also. Presumably a
    // purse...
    //
    return newPurse;
}

std::int32_t MadeEasy::depositCashPurse(
    const std::string& notaryID,
    const std::string& instrumentDefinitionID,
    const std::string& nymID,
    const std::string& oldPurse,
    const std::vector<std::string>& selectedTokens,
    const std::string& accountID,
    bool bReimportIfFailure,  // So we don't re-import a purse that wasn't
                              // internal to begin with.
    std::string* pOptionalOutput /*=nullptr*/) const
{
    rLock lock(lock_);

    std::string recipientNymID = SwigWrap::GetAccountWallet_NymID(accountID);
    if (!VerifyStringVal(recipientNymID)) {
        otOut << "\ndepositCashPurse: Unable to find recipient Nym based on "
                 "myacct. \n";
        return -1;
    }

    bool bPasswordProtected = SwigWrap::Purse_HasPassword(notaryID, oldPurse);

    std::string newPurse;                // being deposited.;
    std::string newPurseForSender = "";  // Probably unused in this case.;
    std::string copyOfOldPurse = oldPurse;
    bool bSuccessProcess = processCashPurse(
        newPurse,
        newPurseForSender,
        notaryID,
        instrumentDefinitionID,
        nymID,
        copyOfOldPurse,
        selectedTokens,
        recipientNymID,
        bPasswordProtected,
        false);

    if (!bSuccessProcess || !VerifyStringVal(newPurse)) {
        otOut << "OT_ME_depositCashPurse: new Purse is empty, after processing "
                 "it for deposit. \n";
        return -1;
    }

    OTAPI_Func theRequest(
        DEPOSIT_CASH,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        accountID,
        newPurse);
    std::string strResponse = theRequest.Run();

    std::string strAttempt = "deposit_cash";

    // HERE, WE INTERPRET THE SERVER REPLY, WHETHER SUCCESS, FAIL, OR ERROR...

    std::int32_t nInterpretReply = InterpretTransactionMsgReply(
        notaryID, recipientNymID, accountID, strAttempt, strResponse);

    if (1 == nInterpretReply) {

        if (nullptr != pOptionalOutput) *pOptionalOutput = strResponse;

        // Download all the intermediary files (account balance, inbox, outbox,
        // etc)
        // since they have probably changed from this operation.
        //
        bool bRetrieved = retrieve_account(
            notaryID,
            recipientNymID,
            accountID,
            true);  // bForceDownload defaults to false.;

        otOut << "\nServer response (" << strAttempt
              << "): SUCCESS depositing cash!\n";
        otOut << std::string(bRetrieved ? "Success" : "Failed")
              << " retrieving intermediary files for account.\n";
    } else  // failure. (so we re-import the cash, so as not to lose it...)
    {

        if (!bPasswordProtected && bReimportIfFailure) {
            bool importStatus = SwigWrap::Wallet_ImportPurse(
                notaryID, instrumentDefinitionID, recipientNymID, newPurse);
            otOut << "Since failure in OT_ME_depositCashPurse, "
                     "OT_API_Wallet_ImportPurse called. Status of "
                     "import: "
                  << importStatus << "\n";

            if (!importStatus) {
                // Raise the alarm here that we failed depositing the purse, and
                // then we failed
                // importing it back into our wallet again.
                otOut << "Error: Failed depositing the cash purse, and then "
                         "failed re-importing it back to wallet. Therefore YOU "
                         "must copy the purse NOW and save it to a safe place! "
                         "\n";

                otOut << newPurse << "\n";

                otOut << "AGAIN: Be sure to copy the above purse "
                         "to a safe place, since it FAILED to "
                         "deposit and FAILED to re-import back "
                         "into the wallet. \n";
            }
        } else {
            otOut << "Error: Failed depositing the cash purse. "
                     "Therefore YOU must copy the purse NOW and "
                     "save it to a safe place! \n";

            otOut << newPurse << "\n";

            otOut << "AGAIN: Be sure to copy the above purse to a "
                     "safe place, since it FAILED to deposit. \n";
        }

        return -1;
    }

    //
    // Return status to caller.
    //
    return nInterpretReply;
}

bool MadeEasy::exchangeCashPurse(
    const std::string& notaryID,
    const std::string& instrumentDefinitionID,
    const std::string& nymID,
    std::string& oldPurse,
    const std::vector<std::string>& selectedTokens) const
{
    rLock lock(lock_);
    std::string newPurse;
    std::string newPurseForSender = "";  // Probably unused in this case.;

    bool bProcessSuccess = processCashPurse(
        newPurse,
        newPurseForSender,
        notaryID,
        instrumentDefinitionID,
        nymID,
        oldPurse,
        selectedTokens,
        nymID,
        false,
        false);  // bIsPasswordProtected=false;

    if (bProcessSuccess && !VerifyStringVal(newPurse)) {
        otOut << "OT_ME_exchangeCashPurse: Before server OT_API_exchangePurse "
                 "call, new Purse is empty. Returning false.\n";
        return false;
    }

    OTAPI_Func theRequest(
        EXCHANGE_CASH,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        instrumentDefinitionID,
        newPurse);
    std::string strResponse = theRequest.Run();

    if (!VerifyStringVal(strResponse)) {
        otOut << "IN OT_ME_exchangeCashPurse: theRequest.SendTransaction(() "
                 "failed. (I give up.) \n";

        bool importStatus = SwigWrap::Wallet_ImportPurse(
            notaryID, instrumentDefinitionID, nymID, newPurse);
        otOut << "Since failure in OT_ME_exchangeCashPurse, "
                 "OT_API_Wallet_ImportPurse called, status of import: "
              << std::string(importStatus ? "true" : "false") << "\n";
        if (!importStatus) {
            //          Utility.setObj(newPurse)
        }

        return false;
    }

    //  otOut << "OT_ME_exchangeCashPurse ends, status:
    // success.\n")

    return true;
}

std::string MadeEasy::deposit_purse(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    const std::string& STR_PURSE) const
{
    rLock lock(lock_);
    OTAPI_Func theRequest(
        DEPOSIT_CASH,
        wallet_,
        Identifier(nymID),
        Identifier(notaryID),
        exec_,
        otapi_,
        ACCT_ID,
        STR_PURSE);

    return theRequest.Run();
}
#endif  // OT_CASH
}  // namespace opentxs

/*

DONE:  create nym, register nym, issue instrument definition, send transfer,
accept entire
inbox, write cheque.

Next ones:  show purse, withdraw cash, deposit cash, withdraw voucher, deposit
cheque.

Need to add functions (like check_nym above) for all of these:

attr OTAPI_Func::REGISTER_NYM (register nym)    DONE
attr OTAPI_Func::DELETE_NYM
attr OTAPI_Func::CHECK_NYM                      DONE
attr OTAPI_Func::SEND_USER_MESSAGE              DONE
attr OTAPI_Func::ISSUE_ASSET_TYPE               DONE
attr OTAPI_Func::ISSUE_BASKET                   DONE
attr OTAPI_Func::CREATE_ASSET_ACCT              DONE
attr OTAPI_Func::DELETE_ASSET_ACCT
attr OTAPI_Func::EXCHANGE_BASKET                DONE
attr OTAPI_Func::PROCESS_INBOX                  DONE
attr OTAPI_Func::DEPOSIT_CASH                   DONE
attr OTAPI_Func::EXCHANGE_CASH
attr OTAPI_Func::DEPOSIT_CHEQUE                 DONE
attr OTAPI_Func::WITHDRAW_VOUCHER               DONE
attr OTAPI_Func::WITHDRAW_CASH                  DONE
attr OTAPI_Func::PAY_DIVIDEND                   DONE
attr OTAPI_Func::GET_CONTRACT                   DONE
attr OTAPI_Func::SEND_TRANSFER                  DONE
attr OTAPI_Func::GET_MARKET_LIST                DONE
attr OTAPI_Func::CREATE_MARKET_OFFER            DONE
attr OTAPI_Func::KILL_MARKET_OFFER              DONE
attr OTAPI_Func::KILL_PAYMENT_PLAN              DONE
attr OTAPI_Func::GET_NYM_MARKET_OFFERS          DONE
attr OTAPI_Func::GET_MARKET_OFFERS              DONE
attr OTAPI_Func::GET_MARKET_RECENT_TRADES
attr OTAPI_Func::GET_MINT                       DONE
attr OTAPI_Func::QUERY_ASSET_TYPES              DONE
attr OTAPI_Func::GET_BOX_RECEIPT                DONE

--- Activate Payment Plan

------ TANGENT -----------------------------------------
Use cases that do not require a server message, and thus will have
representative
functions in ot_made_easy.ot EVEN WHEN there is no corresponding server message
in ot.
--- stat wallet               DONE
--- Stat cash purse           DONE
--- Stat account              DONE
--- Stat account inbox        DONE
--- Stat account outbox       DONE
--- Stat payment inbox        DONE
--- Stat record box           DONE
--- encode / decode           DONE
--- encrypt/decrypt           DONE
--- Password encrypt/decrypt  DONE
--- Sign / Verify             DONE
--- Create asset contract     DONE
--- Create server contract    DONE
--- Create symmetric key      DONE
--- Create pseudonym          DONE
--- Write cheque              DONE
--- Verify last acct receipt  DONE
--- Refresh (download latest) DONE
--- Refresh Nym               DONE
--- Propose Payment Plan
--- Confirm Payment Plan
--- Balance                   DONE
---

------ END TANGENT -------------------------------------


Here are parameters for the first group above.
(They are called in OTAPI_Func, this code is from there):

else if (funcType == DELETE_NYM)
{ SwigWrap::unregisterNym(notaryID, nymID); }
else if (funcType == GET_NYM_MARKET_OFFERS)
{ SwigWrap::getNymMarketOffers(notaryID, nymID); }
else if (funcType == CREATE_ASSET_ACCT)
{ SwigWrap::registerAccount(notaryID, nymID, instrumentDefinitionID); }
else if (funcType == DELETE_ASSET_ACCT)
{ SwigWrap::unregisterAccount(notaryID, nymID, accountID); }
else if (funcType == EXCHANGE_BASKET)
{ SwigWrap::exchangeBasket(notaryID, nymID, instrumentDefinitionID, basket,
bBool); }
else if (funcType == GET_CONTRACT)
{ SwigWrap::getInstrumentDefinition(notaryID, nymID, instrumentDefinitionID);
}
else if (funcType == ISSUE_ASSET_TYPE)
{ SwigWrap::registerInstrumentDefinition(notaryID, nymID, strData); }
else if (funcType == ISSUE_BASKET)
{ SwigWrap::issueBasket(notaryID, nymID, basket); }
else if (funcType == EXCHANGE_CASH)
{ SwigWrap::exchangePurse(notaryID, instrumentDefinitionID, nymID, strData); }
else if (funcType == KILL_MARKET_OFFER)
{ SwigWrap::cancelMarketOffer(notaryID, nymID, accountID, strData); }
else if (funcType == PROCESS_INBOX)
{ SwigWrap::processInbox(notaryID, nymID, accountID, strData); }
else if (funcType == DEPOSIT_CASH)
{ SwigWrap::notarizeDeposit(notaryID, nymID, accountID, strData); }
else if (funcType == DEPOSIT_CHEQUE)
{ SwigWrap::depositCheque(notaryID, nymID, accountID, strData); }
else if (funcType == WITHDRAW_CASH)
{ SwigWrap::notarizeWithdrawal(notaryID, nymID, accountID, strData); }
else if (funcType == WITHDRAW_VOUCHER)
{ SwigWrap::withdrawVoucher(notaryID, nymID, accountID, nymID2, strData,
strData2); }
else if (funcType == SEND_TRANSFER)
{ SwigWrap::notarizeTransfer(notaryID, nymID, accountID, accountID2,
strData, strData2); } // amount and note, for the last two.
else if (funcType == GET_MARKET_LIST)
{ SwigWrap::getMarketList(notaryID, nymID); }
else if (funcType == GET_MARKET_OFFERS)
{ SwigWrap::getMarketOffers(notaryID, nymID, strData, strData2); }
else if (funcType == GET_MARKET_RECENT_TRADES)
{ SwigWrap::getMarketRecentTrades(notaryID, nymID, strData); }
else if (funcType == CREATE_MARKET_OFFER)
{ SwigWrap::issueMarketOffer(notaryID, nymID, instrumentDefinitionID,
accountID, instrumentDefinitionID2,
accountID2,
strData, strData2, strData3, strData4, bBool);
}

*/
