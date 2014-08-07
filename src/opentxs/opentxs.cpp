/************************************************************
 *
 *  opentxs.cpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

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
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#include "opentxs.hpp"

#include <stdafx.hpp>

#include <algorithm>
#include <cctype>

#include <OpenTransactions.hpp>
#include <OTClient.hpp>

#include <OTAccount.hpp>
#include <OTAPI.hpp>
#include <OTAssetContract.hpp>
#include <OTLog.hpp>
#include <OTPaths.hpp>
#include <OTPseudonym.hpp>
#include <OTServerContract.hpp>
#include <OTWallet.hpp>

#include <ot_commands_ot.hpp>
#include <ot_otapi_ot.hpp>

#include <anyoption/anyoption.hpp>

using namespace opentxs;

const char* categoryName[] = {
    "Category Error",           "Advanced utilities",    "The user wallet",
    "Misc",                     "Markets (bid/ask)",     "Asset accounts",
    "Dealing with other users", "Financial instruments", "Basket currencies",
    "Pseudonyms"};

Opentxs::CommandEntry commands[] = {
    {"acceptall",
     OT_Command::mainAcceptAll,
     Opentxs::catAccounts,
     "accept all incoming transfers, receipts, payments, invoices."},
    {"acceptinbox",
     OT_Command::mainAcceptInbox,
     Opentxs::catAccounts,
     "accept all incoming transfers and receipts in MyAcct's inbox."},
    {"acceptinvoices",     OT_Command::mainAcceptInvoices,
     Opentxs::catAccounts, "pay all invoices in MyNym's payments inbox."},
    {"acceptmoney",
     OT_Command::mainAcceptMoney,
     Opentxs::catAccounts,
     "accept all incoming transfers and payments into MyAcct."},
    {"acceptpayments",
     OT_Command::mainAcceptPayments,
     Opentxs::catAccounts,
     "accept all incoming payments in MyNym's payments inbox."},
    {"acceptreceipts",     OT_Command::mainAcceptReceipts,
     Opentxs::catAccounts, "accept all receipts in MyAcct's inbox."},
    {"accepttransfers",    OT_Command::mainAcceptTransfers,
     Opentxs::catAccounts, "accept all incoming transfers in MyAcct's inbox."},
    {"addasset",
     OT_Command::mainAddAsset,
     Opentxs::catWallet,
     "paste an existing asset contract, import into your wallet."},
    {"addserver",
     OT_Command::mainAddServer,
     Opentxs::catWallet,
     "paste an existing server contract, import into your wallet."},
    {"addsignature",
     OT_Command::mainAddSignature,
     Opentxs::catAdmin,
     "add a signature to a contract without releasing others."},
    {"cancel",
     OT_Command::mainCancel,
     Opentxs::catInstruments,
     "cancel an uncashed outgoing instrument from outpayment box."},
    {"changepw",         OT_Command::mainChangePw,
     Opentxs::catWallet, "change the master passphrase for the wallet."},
    {"checknym",             OT_Command::mainCheckNym,
     Opentxs::catOtherUsers, "download a nym's public key based on his ID."},
    {"clearexpired",   OT_Command::mainClearExpired,
     Opentxs::catMisc, "clear all expired records."},
    {"clearrecords",   OT_Command::mainClearRecords,
     Opentxs::catMisc, "clear all archived records and receipts."},
    {"confirm",
     OT_Command::mainConfirm,
     Opentxs::catInstruments,
     "confirm your agreement to a smart contract or payment plan."},
    {"decode",          OT_Command::mainDecode,
     Opentxs::catAdmin, "OT-base64-decode out of armor."},
    {"decrypt",         OT_Command::mainDecrypt,
     Opentxs::catAdmin, "decrypt ciphertext using nym's private key."},
    {"deleteinmail",         OT_Command::mainDeleteInmail,
     Opentxs::catOtherUsers, "delete an in-mail item."},
    {"deleteoutmail",        OT_Command::mainDeleteOutmail,
     Opentxs::catOtherUsers, "delete an out-mail item."},
    {"deposit",            OT_Command::mainDeposit,
     Opentxs::catAccounts, "deposit cash, cheque, voucher, or invoice."},
    {"discard",
     OT_Command::mainDiscard,
     Opentxs::catInstruments,
     "discard an uncashed incoming instrument from payments inbox."},
    {"editaccount",
     OT_Command::mainEditAccount,
     Opentxs::catWallet,
     "edit an asset account label, as it appears in your wallet."},
    {"editasset",
     OT_Command::mainEditAsset,
     Opentxs::catWallet,
     "edit a currency contract label, as it appears in your wallet."},
    {"editnym",          OT_Command::mainEditNym,
     Opentxs::catWallet, "edit the nym label, as it appears in your wallet."},
    {"editserver",
     OT_Command::mainEditServer,
     Opentxs::catWallet,
     "edit a server contract label, as it appears in your wallet."},
    {"encode",          OT_Command::mainEncode,
     Opentxs::catAdmin, "OT-base64-encode into armor."},
    {"encrypt",         OT_Command::mainEncrypt,
     Opentxs::catAdmin, "encrypt plaintext to a nym's public key."},
    {"exchangebasket",    OT_Command::mainExchangeBasket,
     Opentxs::catBaskets, "exchange in/out of a basket currency."},
    {"exportcash",            OT_Command::mainExportCash,
     Opentxs::catInstruments, "export a cash purse."},
    {"exportnym",        OT_Command::mainExportNym,
     Opentxs::catWallet, "export an OT Nym as a single importable file."},
    {"getcontract",     OT_Command::mainGetContract,
     Opentxs::catAdmin, "download an asset or server contract by its ID."},
    {"getmarkets",        OT_Command::mainGetMarkets,
     Opentxs::catMarkets, "download the list of markets."},
    {"getmyoffers",       OT_Command::mainGetMyOffers,
     Opentxs::catMarkets, "download mynym's list of market offers."},
    {"getoffers",         OT_Command::mainGetOffers,
     Opentxs::catMarkets, "download the list of market offers."},
    {"getreceipt",      OT_Command::mainGetReceipt,
     Opentxs::catAdmin, "downloads a box receipt based on transaction ID."},
    {"importcash",            OT_Command::mainImportCash,
     Opentxs::catInstruments, "import a cash purse."},
    {"importnym",        OT_Command::mainImportNym,
     Opentxs::catWallet, "import an OT Nym that was previously exported."},
    {"inbox",              OT_Command::mainInbox,
     Opentxs::catAccounts, "show inbox of a particular account."},
    {"inmail",               OT_Command::mainInmail,
     Opentxs::catOtherUsers, "show in-mail for a particular nym."},
    {"inpayments",           OT_Command::mainInpayments,
     Opentxs::catOtherUsers, "show contents of incoming payments box."},
    {"issueasset",      OT_Command::mainIssueAsset,
     Opentxs::catAdmin, "issue a currency contract onto an OT server."},
    {"killoffer",         OT_Command::mainKillOffer,
     Opentxs::catMarkets, "kill an active recurring market offer."},
    {"killplan",              OT_Command::mainKillPlan,
     Opentxs::catInstruments, "kill an active recurring payment plan."},
    {"newaccount",         OT_Command::mainNewAccount,
     Opentxs::catAccounts, "create a new asset account."},
    {"newasset",        OT_Command::mainNewAsset,
     Opentxs::catAdmin, "create a new asset contract."},
    {"newbasket",         OT_Command::mainNewBasket,
     Opentxs::catBaskets, "create a new basket currency."},
    {"newcredential",  OT_Command::mainNewCredential,
     Opentxs::catNyms, "create a new credential for a specific nym."},
    {"newkey",          OT_Command::mainNewKey,
     Opentxs::catAdmin, "create a new symmetric key."},
    {"newnym", OT_Command::mainNewNym, Opentxs::catNyms, "create a new nym."},
    {"newoffer",          OT_Command::mainNewOffer,
     Opentxs::catMarkets, "create a new market offer."},
    {"newserver",       OT_Command::mainNewServer,
     Opentxs::catAdmin, "create a new server contract."},
    {"outbox",             OT_Command::mainOutbox,
     Opentxs::catAccounts, "show outbox of a particular account."},
    {"outmail",              OT_Command::mainOutmail,
     Opentxs::catOtherUsers, "show out-mail for a particular nym."},
    {"outpayment",           OT_Command::mainOutpayment,
     Opentxs::catOtherUsers, "show contents of outgoing payments box."},
    {"passworddecrypt", OT_Command::mainPasswordDecrypt,
     Opentxs::catAdmin, "password-decrypt a ciphertext using a symmetric key."},
    {"passwordencrypt", OT_Command::mainPasswordEncrypt,
     Opentxs::catAdmin, "password-encrypt a plaintext using a symmetric key."},
    {"paydividend",
     OT_Command::mainPayDividend,
     Opentxs::catMarkets,
     "dividend payout, sent to all shareholders (in voucher form.)"},
    {"payinvoice",           OT_Command::mainPayInvoice,
     Opentxs::catOtherUsers, "pay an invoice."},
    {"proposeplan",
     OT_Command::mainProposePlan,
     Opentxs::catInstruments,
     "as merchant, propose a payment plan to a customer."},
    {"refresh",          OT_Command::mainRefresh,
     Opentxs::catWallet, "performs both refreshnym and refreshaccount."},
    {"refreshaccount",     OT_Command::mainRefreshAccount,
     Opentxs::catAccounts, "download latest intermediary files for myacct."},
    {"refreshnym",     OT_Command::mainRefreshNym,
     Opentxs::catNyms, "download latest intermediary files for mynym."},
    {"registernym",     OT_Command::mainRegisterNym,
     Opentxs::catAdmin, "register a nym onto an OT server."},
    {"revokecredential", OT_Command::mainRevokeCredential,
     Opentxs::catNyms,   "revoke one of a nym's credentials."},
    {"sendcash",
     OT_Command::mainSendCash,
     Opentxs::catOtherUsers,
     "send cash from mypurse to recipient, withdraw if necessary."},
    {"sendcheque",
     OT_Command::mainSendCheque,
     Opentxs::catOtherUsers,
     "write a cheque and then send it to the recipient."},
    {"sendinvoice",
     OT_Command::mainSendInvoice,
     Opentxs::catOtherUsers,
     "write an invoice and then send it to the recipient."},
    {"sendmessage",          OT_Command::mainSendMessage,
     Opentxs::catOtherUsers, "send a message to another nym's in-mail."},
    {"sendvoucher",
     OT_Command::mainSendVoucher,
     Opentxs::catOtherUsers,
     "withdraw a voucher and then send it to the recipient."},
    {"showaccount",        OT_Command::mainShowAccount,
     Opentxs::catAccounts, "show account stats for a single account."},
    {"showaccounts",     OT_Command::mainShowAccounts,
     Opentxs::catWallet, "show the asset accounts in the wallet."},
    {"showactive",
     OT_Command::mainShowActive,
     Opentxs::catInstruments,
     "show the active cron item IDs, or the details of one by ID."},
    {"showassets",       OT_Command::mainShowAssets,
     Opentxs::catWallet, "show the currency contracts in the wallet."},
    {"showbalance",        OT_Command::mainShowBalance,
     Opentxs::catAccounts, "show balance for a specific account."},
    {"showbasket",        OT_Command::mainShowBasket,
     Opentxs::catBaskets, "show basket currencies available in the wallet."},
    {"showcredential", OT_Command::mainShowCredential,
     Opentxs::catNyms, "show a specific credential in detail."},
    {"showcredentials", OT_Command::mainShowCredentials,
     Opentxs::catNyms,  "show the credentials for a specific nym."},
    {"showexpired",    OT_Command::mainShowExpired,
     Opentxs::catMisc, "show contents of expired record box."},
    {"showmarkets",       OT_Command::mainShowMarkets,
     Opentxs::catMarkets, "show the list of markets."},
    {"showmint",
     OT_Command::mainShowMint,
     Opentxs::catAdmin,
     "show mint file for specific asset ID. Download if necessary."},
    {"showmyoffers",
     OT_Command::mainShowMyOffers,
     Opentxs::catMarkets,
     "show mynym's offers on a particular server and market."},
    {"shownym",        OT_Command::mainShowNym,
     Opentxs::catNyms, "show the statistics for a specific nym."},
    {"shownyms",         OT_Command::mainShowNyms,
     Opentxs::catWallet, "show the nyms in the wallet."},
    {"showoffers",        OT_Command::mainShowOffers,
     Opentxs::catMarkets, "show all offers on a particular server and market."},
    {"showpayment",
     OT_Command::mainShowPayment,
     Opentxs::catOtherUsers,
     "show the details of an incoming payment in the payments inbox."},
    {"showpurse",        OT_Command::mainShowPurse,
     Opentxs::catWallet, "show contents of cash purse."},
    {"showrecords",    OT_Command::mainShowRecords,
     Opentxs::catMisc, "show contents of record box."},
    {"showservers",      OT_Command::mainShowServers,
     Opentxs::catWallet, "show the server contracts in the wallet."},
    {"showwallet",       OT_Command::mainShowWallet,
     Opentxs::catWallet, "show wallet contents."},
    {"signcontract",
     OT_Command::mainSignContract,
     Opentxs::catAdmin,
     "sign a contract, releasing all other signatures first."},
    {"transfer",           OT_Command::mainTransfer,
     Opentxs::catAccounts, "send a transfer from myacct to hisacct."},
    {"triggerclause",         OT_Command::mainTriggerClause,
     Opentxs::catInstruments, "trigger a clause on a running smart contract."},
    {"verifyreceipt",
     OT_Command::mainVerifyReceipt,
     Opentxs::catAccounts,
     "verify your intermediary files against last signed receipt."},
    {"verifysignature", OT_Command::mainVerifySignature,
     Opentxs::catAdmin, "verify a signature on a contract."},
    {"withdraw",
     OT_Command::mainWithdrawCash,
     Opentxs::catInstruments,
     "withdraw cash. (From acct on server into local purse.)"},
    {"withdrawvoucher",
     OT_Command::mainWithdrawVoucher,
     Opentxs::catInstruments,
     "withdraw from myacct as a voucher (cashier's cheque.)"},
    {"writecheque",           OT_Command::mainWriteCheque,
     Opentxs::catInstruments, "write a cheque and print it out to the screen."},
    {"writeinvoice",
     OT_Command::mainWriteInvoice,
     Opentxs::catInstruments,
     "write an invoice and print it out to the screen."},
    {nullptr, nullptr, Opentxs::catError, nullptr}};

Opentxs::Opentxs()
{
    OTAPI_Wrap::AppInit();
}

Opentxs::~Opentxs()
{
    OTAPI_Wrap::AppCleanup();
}

std::string& Opentxs::ltrim(std::string& s)
{
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

std::string& Opentxs::rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(
                                                   std::isspace))).base(),
            s.end());
    return s;
}

std::string& Opentxs::trim(std::string& s)
{
    return ltrim(rtrim(s));
}

void Opentxs::handleCommandLineArguments(int argc, char* argv[], AnyOption& opt)
{
    OTString configPath(OTPaths::AppDataFolder());
    bool configPathFound = configPath.Exists() && 3 < configPath.GetLength();
    OT_ASSERT_MSG(configPathFound,
                  "RegisterAPIWithScript: Must set Config Path first!\n");
    OTLog::vOutput(1, "Using configuration path:  %s\n", configPath.Get());

    opt.addUsage("");
    opt.addUsage(" Opentxs CLI Usage:  ");
    opt.addUsage("");

    opt.setCommandFlag("echocommand");
    opt.setCommandFlag("echoexpand");
    opt.setCommandFlag("errorlist");
    opt.setCommandFlag("noprompt");
    opt.setCommandFlag("test");

    opt.setCommandOption("args");
    opt.setCommandOption("myacct");
    opt.setCommandOption("mynym");
    opt.setCommandOption("mypurse");
    opt.setCommandOption("hisacct");
    opt.setCommandOption("hisnym");
    opt.setCommandOption("hispurse");
    opt.setCommandOption("server");

    // for options that will be checked only from the CLI option file
    opt.setFileOption("defaultserver");
    opt.setFileOption("defaultmyacct");
    opt.setFileOption("defaultmynym");
    opt.setFileOption("defaultmypurse");
    opt.setFileOption("defaulthisacct");
    opt.setFileOption("defaulthisnym");
    opt.setFileOption("defaulthispurse");

    OTString optionsFile("command-line-ot.opt"), iniFileExact;
    bool buildFullPathSuccess =
        OTPaths::RelativeToCanonical(iniFileExact, configPath, optionsFile);
    OT_ASSERT_MSG(buildFullPathSuccess, "Unalbe to set Full Path");

    opt.processFile(iniFileExact.Get());
    opt.processCommandArgs(argc, argv);
}

const char* Opentxs::getOption(AnyOption& opt, const char* defaultName,
                               const char* optionName)
{
    // can we get the default value from the command line?
    const char* value = opt.getValue(optionName);
    if (value != nullptr) {
        OTLog::vOutput(1, "Option  %s: %s\n", optionName, value);
        return value;
    }

    // can we get the default value from the options file?
    value = opt.getValue(defaultName);
    if (value != nullptr) {
        OTLog::vOutput(1, "Default %s: %s\n", optionName, value);
        return value;
    }

    // clear option value
    return "";
}

OTVariable* Opentxs::setGlobalVariable(OT_ME& madeEasy, const std::string& name,
                                       const std::string& value)
{
    if (value.size() == 0) {
        OTLog::vOutput(2, "Variable %s isn't set\n", name.c_str());
        return nullptr;
    }

    OTLog::vOutput(1, "Variable %s has value: %s\n", name.c_str(),
                   value.c_str());

    OTVariable* var = new OTVariable(name, value, OTVariable::Var_Constant);
    OT_ASSERT(var != nullptr);
    madeEasy.AddVariable(name, *var);
    return var;
}

int Opentxs::processCommand(OT_ME& madeEasy, AnyOption& opt)
{
    // process command line values such as account ID, Nym ID, etc.
    // Also available as defaults in a config file in the ~/.ot folder
    args_ = getOption(opt, "defaultargs", "args");
    hisAcct_ = getOption(opt, "defaulthisacct", "hisacct");
    hisNym_ = getOption(opt, "defaulthisnym", "hisnym");
    hisPurse_ = getOption(opt, "defaulthispurse", "hispurse");
    myAcct_ = getOption(opt, "defaultmyacct", "myacct");
    myNym_ = getOption(opt, "defaultmynym", "mynym");
    myPurse_ = getOption(opt, "defaultmypurse", "mypurse");
    server_ = getOption(opt, "defaultserver", "server");

    OTWallet* wallet = OTAPI_Wrap::OTAPI()->GetWallet();

    OT_ASSERT_MSG(
        wallet != nullptr,
        "The wallet object is still nullptr, somehow. Please load it.\n");

    OTServerContract* serverContract = nullptr;
    if (server_.size() > 0) {
        serverContract = wallet->GetServerContract(server_);
        if (serverContract == nullptr) {
            serverContract = wallet->GetServerContractPartialMatch(server_);
            if (serverContract == nullptr) {
                OTLog::vOutput(
                    0, "Unknown default server contract for --server %s\n",
                    server_.c_str());
            }
        }
        if (serverContract != nullptr) {
            OTString tmp;
            serverContract->GetIdentifier(tmp);
            server_ = tmp.Get();
            OTLog::vOutput(0, "Using as server: %s\n", server_.c_str());
        }
    }

    OTPseudonym* myNym = nullptr;
    if (myNym_.size() > 0) {
        myNym = wallet->GetNymByID(myNym_);
        if (myNym == nullptr) {
            myNym = wallet->GetNymByIDPartialMatch(myNym_);
            if (myNym == nullptr) {
                OTLog::vOutput(0, "Unknown default nym for --mynym %s\n",
                               myNym_.c_str());
            }
        }
        if (myNym != nullptr) {
            OTString tmp;
            myNym->GetIdentifier(tmp);
            myNym_ = tmp.Get();
            OTLog::vOutput(0, "Using as mynym: %s\n", myNym_.c_str());
        }
    }

    OTAccount* myAccount = nullptr;
    if (myAcct_.size() > 0) {
        myAccount = wallet->GetAccount(myAcct_);
        if (myAccount == nullptr) {
            myAccount = wallet->GetAccountPartialMatch(myAcct_);
            if (myAccount == nullptr) {
                OTLog::vOutput(0, "Unknown default account for --myacct %s\n",
                               myAcct_.c_str());
            }
        }
        if (myAccount != nullptr) {
            OTString tmp;
            myAccount->GetPurportedAccountID().GetString(tmp);
            myAcct_ = tmp.Get();
            OTLog::vOutput(0, "Using as myacct: %s\n", myAcct_.c_str());
        }
    }

    OTPseudonym* hisNym = nullptr;
    if (hisNym_.size() > 0) {
        hisNym = wallet->GetNymByID(hisNym_);
        if (hisNym == nullptr) {
            hisNym = wallet->GetNymByIDPartialMatch(hisNym_);
            if (hisNym == nullptr) {
                OTLog::vOutput(0, "Unknown default nym for --hisnym %s\n",
                               hisNym_.c_str());
            }
        }
        if (hisNym != nullptr) {
            OTString tmp;
            hisNym->GetIdentifier(tmp);
            hisNym_ = tmp.Get();
            OTLog::vOutput(0, "Using as hisnym: %s\n", hisNym_.c_str());
        }
    }

    OTAccount* hisAccount = nullptr;
    if (hisAcct_.size() > 0) {
        hisAccount = wallet->GetAccount(hisAcct_);
        if (hisAccount == nullptr) {
            hisAccount = wallet->GetAccountPartialMatch(hisAcct_);
            if (hisAccount == nullptr) {
                OTLog::vOutput(0, "Unknown default account for --hisacct %s\n",
                               hisAcct_.c_str());
            }
        }
        if (hisAccount != nullptr) {
            OTString tmp;
            hisAccount->GetPurportedAccountID().GetString(tmp);
            hisAcct_ = tmp.Get();
            OTLog::vOutput(0, "Using as hisacct: %s\n", hisAcct_.c_str());
        }
    }

    OTIdentifier purseAssetTypeID;
    OTAssetContract* myAssetContract = nullptr;
    if (myPurse_.size() > 0) {
        myAssetContract = wallet->GetAssetContract(myPurse_);
        if (myAssetContract == nullptr) {
            myAssetContract = wallet->GetAssetContractPartialMatch(myPurse_);
            if (myAssetContract == nullptr) {
                OTLog::vOutput(0, "Unknown default purse for --mypurse %s\n",
                               myPurse_.c_str());
            }
        }
        if (myAssetContract != nullptr) {
            myAssetContract->GetIdentifier(purseAssetTypeID);
            OTString tmp;
            myAssetContract->GetIdentifier(tmp);
            myPurse_ = tmp.Get();
            OTLog::vOutput(0, "Using as mypurse: %s\n", myPurse_.c_str());
        }
    }

    OTIdentifier hisPurseAssetTypeID;
    OTAssetContract* hisAssetContract = nullptr;
    if (hisPurse_.size() > 0) {
        hisAssetContract = wallet->GetAssetContract(hisPurse_);
        if (hisAssetContract == nullptr) {
            hisAssetContract = wallet->GetAssetContractPartialMatch(hisPurse_);
            if (hisAssetContract == nullptr) {
                OTLog::vOutput(0, "Unknown default purse for --hispurse %s\n",
                               hisPurse_.c_str());
            }
        }
        if (hisAssetContract != nullptr) {
            hisAssetContract->GetIdentifier(hisPurseAssetTypeID);
            OTString tmp;
            hisAssetContract->GetIdentifier(tmp);
            hisPurse_ = tmp.Get();
            OTLog::vOutput(0, "Using as hispurse: %s\n", hisPurse_.c_str());
        }
    }

    OTLog::Output(0, "\n");

    if (serverContract != nullptr && myNym != nullptr) {
        OTAPI_Wrap::OTAPI()->GetClient()->SetFocusToServerAndNym(
            *serverContract, *myNym,
            OTAPI_Wrap::OTAPI()->GetTransportCallback());
    }

    string command = "list";
    if (opt.getArgc() == 1) {
        command = opt.getArgv(0);
    }
    else {
        OTLog::vOutput(0, "Expecting a single opentxs command:\n\n");
    }

    typedef std::unique_ptr<OTVariable> GlobalVar;
    GlobalVar varArgs(setGlobalVariable(madeEasy, "Args", args_));
    GlobalVar varMyAcct(setGlobalVariable(madeEasy, "MyAcct", myAcct_));
    GlobalVar varMyNym(setGlobalVariable(madeEasy, "MyNym", myNym_));
    GlobalVar varMyPurse(setGlobalVariable(madeEasy, "MyPurse", myPurse_));
    GlobalVar varHisAcct(setGlobalVariable(madeEasy, "HisAcct", hisAcct_));
    GlobalVar varHisNym(setGlobalVariable(madeEasy, "HisNym", hisNym_));
    GlobalVar varHisPurse(setGlobalVariable(madeEasy, "HisPurse", hisPurse_));
    GlobalVar varServer(setGlobalVariable(madeEasy, "Server", server_));

    OTAPI_Func::CopyVariables();

    OTLog::Output(1, "Script output:\n\n");

    int result = opentxsCommand(command);
    return opt.getArgc() == 1 ? result : -2;
}

int Opentxs::opentxsCommand(const string& command)
{
    if ("exit" == command || "quit" == command) {
        return -2;
    }

    if ("list" == command) {
        OTAPI_Wrap::Output(0, "\nCommands:\n\n");
        for (int32_t i = 0; commands[i].command != nullptr; i++) {
            CommandEntry& cmd = commands[i];
            OTAPI_Wrap::Output(0, (cmd.command + spaces18).substr(0, 18));
            if (i % 4 == 3) {
                OTAPI_Wrap::Output(0, "\n");
            }
        }
        OTAPI_Wrap::Output(0, "\n");
        return 0;
    }

    if ("help" == command) {
        // create category groups
        string categoryGroup[catLast];
        for (int i = 1; i < catLast; i++) {
            categoryGroup[i] = string("\n ") + categoryName[i] + ":\n";
        }

        // add commands to their category group
        OTAPI_Wrap::Output(0, "\nCommands:\n");
        for (int32_t i = 0; commands[i].command != nullptr; i++) {
            CommandEntry& cmd = commands[i];
            categoryGroup[cmd.category] +=
                (cmd.command + spaces18).substr(0, 18) + cmd.helpText + "\n";
        }

        // print all category groups
        for (int i = 1; i < catLast; i++) {
            OTAPI_Wrap::Output(0, categoryGroup[i]);
        }

        return 0;
    }

    // all other commands.
    for (int32_t i = 0; commands[i].command != nullptr; i++) {
        CommandEntry& cmd = commands[i];
        if (cmd.command == command) {
            int32_t returnValue = (*cmd.function)();
            switch (returnValue) {
            case 0: // no action performed, return success
                return 0;
            case 1: // success
                return 0;
            case -1: // failed
                return -1;
            default: // should not happen
                OTAPI_Wrap::Output(0, "\nUndefined error code: \"" +
                                          std::to_string(returnValue) +
                                          "\".\n\n");
                return -1;
            }
            break;
        }
    }

    OTAPI_Wrap::Output(0, "\nUndefined command: \"" + command +
                              "\" -- Try 'list'.\n\n");
    return -1;
}

int Opentxs::run(int argc, char* argv[])
{
    if (OTAPI_Wrap::OTAPI() == nullptr) return -1;

    OTAPI_Wrap::OTAPI()->LoadWallet();

    std::map<std::string, std::string> macros;
    std::vector<int> errorLineNumbers;
    std::vector<std::string> errorCommands;

    OT_ME madeEasy;

    AnyOption opt;
    handleCommandLineArguments(argc, argv, opt);

    if (opt.getArgc() != 0) {
        return processCommand(madeEasy, opt);
    }

    int lineNumber = 0;
    bool echoCommand = opt.getFlag("echocommand") || opt.getFlag("test");
    bool echoExpand = opt.getFlag("echoexpand") || opt.getFlag("test");
    bool noPrompt = opt.getFlag("noprompt") || opt.getFlag("test");
    int processed = 0;
    int failed = 0;
    while (true) {
        // get next command line from input stream
        if (!noPrompt) {
            std::cout << "\nopentxs> ";
        }
        std::string cmd;
        std::getline(std::cin, cmd);

        // end of file stops processing commands
        if (std::cin.eof()) {
            break;
        }

        lineNumber++;

        // quit/exit the command loop?
        cmd = trim(cmd);
        if (echoCommand) {
            std::cout << cmd << std::endl;
        }

        if (cmd == "quit" || cmd == "exit") {
            break;
        }

        // empty lines and lines starting with a hash character are seen as
        // comments
        if (cmd.size() == 0 || cmd[0] == '#') {
            continue;
        }

        std::string originalCmd = cmd;

        // lines starting with a dollar sign character denote the definition of
        // a macro of the form: $macroName = macroValue
        // whitespace around the equal sign is optional
        // <macroName> can be any combination of A-Z, a-z, 0-9, or _
        // <macroValue> is anything after the equal sign and whitespace-trimmed
        // note that <macroValue> can be an empty string
        // note that the dollar sign itself is part of the immediately following
        // macro name
        // note that a macro value stays valid for the entire lifetime of the
        // command loop
        // note that macro expansion is done on the command line before
        // processing the line this means that a macro is allowed to contain
        // command line escape characters
        // note that macro expansion is recursive until no expansions are found
        // any more this means that a macro is allowed to contain other macros
        if (cmd[0] == '$') {
            // determine the macro name
            size_t nameLength = 1;
            while (nameLength < cmd.length() &&
                   (std::isalnum(cmd[nameLength]) || cmd[nameLength] == '_')) {
                nameLength++;
            }
            std::string macroName = cmd.substr(0, nameLength);

            // skip whitespace
            size_t i = nameLength;
            while (i < cmd.length() && std::isspace(cmd[i])) {
                i++;
            }

            if (i == cmd.length() || cmd[i] != '=') {
                OTLog::vOutput(0, "\n\n***ERROR***\n"
                                  "Expected macro definition of the form: "
                                  "$macroName = macroValue\n"
                                  "Command was: %s",
                               cmd.c_str());
                continue;
            }

            // remainder of line after trimming whitespace is macro value
            std::string macroValue = cmd.substr(i + 1);
            macros[macroName] = trim(macroValue);
            continue;
        }

        // now replace any macro in the command line with its value
        // unknown macro names will cause an error message instead of command
        // execution
        // note that all macro names are 'maximum munch'
        int expansions = 0;
        for (size_t macro = cmd.find_first_of("$"); macro != std::string::npos;
             macro = cmd.find_first_of("$", macro + 1)) {
            // first see if this is an escaped literal
            if (macro > 0 && cmd[macro - 1] == '\\') {
                continue;
            }

            // gather rest of macro name 'maximum munch'
            size_t macroEnd = macro + 1;
            while (macroEnd < cmd.length() &&
                   (std::isalnum(cmd[macroEnd]) || cmd[macroEnd] == '_')) {
                macroEnd++;
            }

            // has this macro been defined?
            std::string macroName = cmd.substr(macro, macroEnd - macro);
            std::map<std::string, std::string>::iterator found =
                macros.find(macroName);
            if (found == macros.end()) {
                OTLog::vOutput(0, "\n\n***ERROR***\n"
                                  "Macro expansion failed.\n"
                                  "Unknown macro: %s\n"
                                  "Command was: %s",
                               macroName.c_str(), cmd.c_str());
                expansions = 100;
                break;
            }

            std::string& macroValue = found->second;

            // limit to 100 expansions to avoid endless recusion loop
            expansions++;
            if (expansions > 100) {
                OTLog::vOutput(0, "\n\n***ERROR***\n"
                                  "Macro expansion failed.\n"
                                  "Too many expansions at macro: %s\n"
                                  "Command was: %s",
                               macroName.c_str(), cmd.c_str());
                break;
            }

            // limit to 10000 characters to avoid crazy recursive expansions
            if (cmd.length() + macroValue.length() > 10000) {
                OTLog::vOutput(0, "\n\n***ERROR***\n"
                                  "Macro expansion failed.\n"
                                  "Command length exceeded at macro: %s\n"
                                  "Macro value is: %s\n"
                                  "Command was: %s",
                               macroName.c_str(), macroValue.c_str(),
                               cmd.c_str());
                expansions = 100;
                break;
            }

            // expand the command line
            cmd = cmd.substr(0, macro) + macroValue + cmd.substr(macroEnd);
        }

        if (echoExpand && cmd != originalCmd) {
            std::cout << cmd << std::endl;
        }

        // skip command when anything during macro expansion failed
        if (expansions > 99) {
            continue;
        }

        // '!' indicates that we expect this command to fail
        //     which is very useful for running a test script
        bool expectFailure = cmd[0] == '!';

        // Parse command string into its separate parts so it can be passed as
        // an argc/argv combo
        // Whitespace separates args as usual.
        // To include whitespace in an arg surround the entire arg with double
        // quotes
        // An unterminated double-quoted arg will auto-terminate at end of line
        // All characters are taken literal except for: double quote, dollar
        // sign, and backslash
        // To take any character literal, precede it with a backslash
        std::vector<std::string> arguments;

        // add original command name
        arguments.push_back(argv[0]);

        // set up buffer that will receive the separate arguments
        char* buf = new char[cmd.length() + 1];
        char* arg = buf;

        // start at actual command
        size_t i = expectFailure ? 1 : 0;
        while (i < cmd.length()) {
            // skip any whitespace
            while (i < cmd.length() && std::isspace(cmd[i])) {
                i++;
            }
            if (i == cmd.length()) {
                // it was trailing whitespace; we're done
                break;
            }

            // remember where we start this new argument in the buffer
            char* start = arg;

            // unquoted argument?
            if (cmd[i] != '"') {
                // take everything until end of line or next whitespace
                while (i < cmd.length() && !std::isspace(cmd[i])) {
                    // unescaped literal character?
                    if (cmd[i] != '\\') {
                        // yep, add to buffer and go for next
                        *arg++ = cmd[i++];
                        continue;
                    }

                    // take next character literal unless it was the end of line
                    // in which case we simply add the backslash as a literal
                    // character
                    *arg++ = i < cmd.length() ? cmd[i++] : '\\';
                }

                // end of argument reached, terminate an add to arguments array
                *arg++ = '\0';
                arguments.push_back(start);

                // look for next argument
                continue;
            }

            // double quoted argument, skip the quote
            i++;

            // take everything until end of line or next double quote
            while (i < cmd.length() && cmd[i] != '"') {
                // unescaped literal character?
                if (cmd[i] != '\\') {
                    // yep, add to buffer and go for next
                    *arg++ = cmd[i++];
                    continue;
                }

                // take next character literal unless it was the end of line
                // in which case we simply add the backslash as a literal
                // character
                *arg++ = i < cmd.length() ? cmd[i++] : '\\';
            }

            // end of argument reached, terminate an add to arguments array
            *arg++ = '\0';
            arguments.push_back(start);

            // skip terminating double quote or end of line
            i++;
        }

        // set up a new argc/argv combo
        int newArgc = arguments.size();
        char** newArgv = new char* [newArgc];
        for (int i = 0; i < newArgc; i++) {
            newArgv[i] = const_cast<char*>(arguments[i].c_str());
        }

        // preprocess the command line
        AnyOption opt;
        handleCommandLineArguments(newArgc, newArgv, opt);

        bool bFailedCommand = 0 != processCommand(madeEasy, opt);
        if (expectFailure) {
            if (!bFailedCommand) {
                failed++;
                OTLog::vOutput(0, "\n\n***ERROR***\nExpected command to "
                                  "fail.\nSucceeding command was: %s",
                               cmd.c_str());
                errorLineNumbers.push_back(lineNumber);
                errorCommands.push_back(originalCmd);
            }
        }
        else {
            if (bFailedCommand) {
                failed++;
                OTLog::vOutput(0, "\n\n***ERROR***\nFailed command was: %s",
                               cmd.c_str());
                errorLineNumbers.push_back(lineNumber);
                errorCommands.push_back(originalCmd);
            }
        }

        delete[] newArgv;
        delete[] buf;

        OTLog::Output(0, "\n\n");
        processed++;
    }

    std::cout << "\n\n" << processed << " commands were processed.\n" << failed
              << " commands failed.\n" << std::endl;

    if (opt.getFlag("errorList") || opt.getFlag("test")) {
        for (size_t i = 0; i < errorLineNumbers.size(); i++) {
            std::cout << "\nFailed line " << errorLineNumbers[i] << ": "
                      << errorCommands[i] << std::endl;
        }
    }

    return failed == 0 ? 0 : -1;
}
