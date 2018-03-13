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

#include "opentxs/client/commands/CmdWithdrawCash.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/Api.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/client/commands/CmdBase.hpp"
#include "opentxs/client/ServerAction.hpp"
#include "opentxs/client/SwigWrap.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/OT.hpp"

#include <stdint.h>
#include <ostream>
#include <string>

using namespace opentxs;
using namespace std;

CmdWithdrawCash::CmdWithdrawCash()
{
    command = "withdraw";
    args[0] = "--myacct <account>";
    args[1] = "--amount <amount>";
    category = catInstruments;
    help = "Withdraw from myacct as cash into local purse.";
}

CmdWithdrawCash::~CmdWithdrawCash() {}

int32_t CmdWithdrawCash::runWithOptions()
{
    return run(getOption("myacct"), getOption("amount"));
}

int32_t CmdWithdrawCash::run(string myacct, string amount)
{
    if (!checkAccount("myacct", myacct)) {
        return -1;
    }

    int64_t value = checkAmount("amount", amount, myacct);
    if (OT_ERROR_AMOUNT == value) {
        return -1;
    }

    return withdrawCash(myacct, value);
}

// CHECK USER (download a public key)
//
std::string CmdWithdrawCash::check_nym(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID) const
{
    auto action = OT::App().API().ServerAction().DownloadNym(
        Identifier(nymID), Identifier(notaryID), Identifier(targetNymID));

    return action->Run();
}

#if OT_CASH
// LOAD MINT (from local storage)
//
// To load a mint withOUT retrieving it from server, call:
//
// var strMint = OT_API_LoadMint(notaryID, instrumentDefinitionID);
// It returns the mint, or null.
// LOAD MINT (from local storage).
// Also, if necessary, RETRIEVE it from the server first.
//
// Returns the mint, or null.
//
std::string CmdWithdrawCash::load_or_retrieve_mint(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& instrumentDefinitionID) const
{
    std::string response = check_nym(notaryID, nymID, nymID);

    if (1 != VerifyMessageSuccess(response)) {
        otOut << "CmdWithdrawCash_load_or_retrieve_mint: Cannot verify nym for "
                 "IDs: \n";
        otOut << "  Notary ID: " << notaryID << "\n";
        otOut << "     Nym ID: " << nymID << "\n";
        otOut << "   Instrument Definition Id: " << instrumentDefinitionID
              << "\n";
        return "";
    }

    // HERE, WE MAKE SURE WE HAVE THE PROPER MINT...
    //
    // Download the public mintfile if it's not there, or if it's expired.
    // Also load it up into memory as a std::string (just to make sure it
    // works.)

    // expired or missing.
    if (!SwigWrap::Mint_IsStillGood(notaryID, instrumentDefinitionID)) {
        otWarn << "CmdWithdrawCash_load_or_retrieve_mint: Mint file is "
                  "missing or expired. Downloading from "
                  "server...\n";

        response = OT::App()
                       .API()
                       .ServerAction()
                       .DownloadMint(
                           Identifier(nymID),
                           Identifier(notaryID),
                           Identifier(instrumentDefinitionID))
                       ->Run();

        if (1 != VerifyMessageSuccess(response)) {
            otOut << "CmdWithdrawCash_load_or_retrieve_mint: Unable to "
                     "retrieve mint for IDs: \n";
            otOut << "  Notary ID: " << notaryID << "\n";
            otOut << "     Nym ID: " << nymID << "\n";
            otOut << "   Instrument Definition Id: " << instrumentDefinitionID
                  << "\n";
            return "";
        }

        if (!SwigWrap::Mint_IsStillGood(notaryID, instrumentDefinitionID)) {
            otOut << "CmdWithdrawCash_load_or_retrieve_mint: Retrieved "
                     "mint, but still 'not good' for IDs: \n";
            otOut << "  Notary ID: " << notaryID << "\n";
            otOut << "     Nym ID: " << nymID << "\n";
            otOut << "   Instrument Definition Id: " << instrumentDefinitionID
                  << "\n";
            return "";
        }
    }
    // else // current mint IS available already on local storage (and not
    // expired.)

    // By this point, the mint is definitely good, whether we had to download it
    // or not.
    // It's here, and it's NOT expired. (Or we would have returned already.)

    std::string strMint = SwigWrap::LoadMint(notaryID, instrumentDefinitionID);
    if (!VerifyStringVal(strMint)) {
        otOut << "CmdWithdrawCash_load_or_retrieve_mint: Unable to load mint "
                 "for IDs: \n";
        otOut << "  Notary ID: " << notaryID << "\n";
        otOut << "     Nym ID: " << nymID << "\n";
        otOut << "   Instrument Definition Id: " << instrumentDefinitionID
              << "\n";
    }

    return strMint;
}
#endif  // OT_CASH

int32_t CmdWithdrawCash::withdrawCash(const string& myacct, int64_t amount)
    const
{
#if OT_CASH
    string server = SwigWrap::GetAccountWallet_NotaryID(myacct);
    if ("" == server) {
        otOut << "Error: cannot determine server from myacct.\n";
        return -1;
    }

    string mynym = SwigWrap::GetAccountWallet_NymID(myacct);
    if ("" == mynym) {
        otOut << "Error: cannot determine mynym from myacct.\n";
        return -1;
    }

    string assetType = getAccountAssetType(myacct);
    if ("" == assetType) {
        return -1;
    }

    const Identifier theNotaryID{server}, theNymID{mynym},
        theAssetType{assetType}, theAcctID{myacct};

    string assetContract = SwigWrap::GetAssetType_Contract(assetType);
    if ("" == assetContract) {
        string response =
            OT::App()
                .API()
                .ServerAction()
                .DownloadContract(theNymID, theNotaryID, theAssetType)
                ->Run();
        if (1 != responseStatus(response)) {
            otOut << "Error: cannot retrieve asset contract.\n";
            return -1;
        }

        assetContract = SwigWrap::GetAssetType_Contract(assetType);
        if ("" == assetContract) {
            otOut << "Error: cannot load asset contract.\n";
            return -1;
        }
    }

    string mint = load_or_retrieve_mint(server, mynym, assetType);
    if ("" == mint) {
        otOut << "Error: cannot load asset mint.\n";
        return -1;
    }

    string response =
        OT::App()
            .API()
            .ServerAction()
            .WithdrawCash(theNymID, theNotaryID, theAcctID, amount)
            ->Run();
    int32_t reply =
        responseReply(response, server, mynym, myacct, "withdraw_cash");
    if (1 != reply) {
        return reply;
    }

    if (!OT::App().API().ServerAction().DownloadAccount(
            theNymID, theNotaryID, theAcctID, true)) {
        otOut << "Error retrieving intermediary files for account.\n";
        return -1;
    }

    return 1;
#else
    return -1;
#endif  // OT_CASH
}
