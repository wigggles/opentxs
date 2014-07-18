#include <stdafx.hpp>

#include <string>

#include "OT_ME.hpp"
#include "OTVariable.hpp"

#include "ot_utility_ot.hpp"
#include "ot_otapi_ot.hpp"
#include "ot_commands_ot.hpp"

using std::string;

namespace opentxs
{

#define OT_OPENTXS_OT

typedef enum {
    catError = 0,
    catAdmin = 1,
    catWallet = 2,
    catMisc = 3,
    catMarkets = 4,
    catAccounts = 5,
    catOtherUsers = 6,
    catInstruments = 7,
    catBaskets = 8,
    catNyms = 9,
    catLast = 10
} Category;

static string categoryName[] = {
    "Category Error",           "Advanced utilities",    "The user wallet",
    "Misc",                     "Markets (bid/ask)",     "Asset accounts",
    "Dealing with other users", "Financial instruments", "Basket currencies",
    "Pseudonyms"};

typedef struct
{
    string command;
    int32_t (*function)();
    Category category;
    string helpText;
} MapFunction;

MapFunction map_functions[] = {
    {"acceptall",
     OT_Command::mainAcceptAll,
     catAccounts,
     "accept all incoming transfers, receipts, payments, and invoices."},
    {"acceptinbox",
     OT_Command::mainAcceptInbox,
     catAccounts,
     "accept all incoming transfers and receipts in MyAcct's inbox."},
    {"acceptinvoices", OT_Command::mainAcceptInvoices,
     catAccounts,      "pay all invoices in MyNym's payments inbox."},
    {"acceptmoney", OT_Command::mainAcceptMoney,
     catAccounts,   "accept all incoming transfers and payments into MyAcct."},
    {"acceptpayments",
     OT_Command::mainAcceptPayments,
     catAccounts,
     "accept all incoming payments in MyNym's payments inbox."},
    {"acceptreceipts", OT_Command::mainAcceptReceipts,
     catAccounts,      "accept all receipts in MyAcct's inbox."},
    {"accepttransfers", OT_Command::mainAcceptTransfers,
     catAccounts,       "accept all incoming transfers in MyAcct's inbox."},
    {"addasset",
     OT_Command::mainAddAsset,
     catWallet,
     "paste an existing asset contract, import it into your wallet."},
    {"addserver",
     OT_Command::mainAddServer,
     catWallet,
     "paste an existing server contract, import it into your wallet."},
    {"addsignature", OT_Command::mainAddSignature,
     catAdmin,       "add a signature to a contract without releasing others."},
    {"cancel",
     OT_Command::mainCancel,
     catInstruments,
     "cancel a not-yet-cashed, outgoing instrument from outpayment box."},
    {"changepw", OT_Command::mainChangePw,
     catWallet,  "change the master passphrase for the wallet."},
    {"checknym",    OT_Command::mainCheckNym,
     catOtherUsers, "download a nym's public key based on his ID."},
    {"clearexpired", OT_Command::mainClearExpired,
     catMisc,        "clear all expired records."},
    {"clearrecords", OT_Command::mainClearRecords,
     catMisc,        "clear all archived records and receipts."},
    {"confirm",
     OT_Command::mainConfirm,
     catInstruments,
     "confirm your agreement to a smart contract or payment plan."},
    {"decode", OT_Command::mainDecode,
     catAdmin, "OT-base64-decode out of armor."},
    {"decrypt", OT_Command::mainDecrypt,
     catAdmin,  "decrypt ciphertext using nym's private key."},
    {"deleteinmail", OT_Command::mainDeleteInmail,
     catOtherUsers,  "delete an in-mail item."},
    {"deleteoutmail", OT_Command::mainDeleteOutmail,
     catOtherUsers,   "delete an out-mail item."},
    {"deposit",   OT_Command::mainDeposit,
     catAccounts, "deposit cash, cheque, voucher, or invoice."},
    {"discard",
     OT_Command::mainDiscard,
     catInstruments,
     "discard a not-yet-cashed, incoming instrument from payments inbox."},
    {"editaccount",
     OT_Command::mainEditAccount,
     catWallet,
     "edit an asset account's label, as it appears in your wallet."},
    {"editasset",
     OT_Command::mainEditAsset,
     catWallet,
     "edit a currency contract's label, as it appears in your wallet."},
    {"editnym", OT_Command::mainEditNym,
     catWallet, "edit the nym's label, as it appears in your wallet."},
    {"editserver",
     OT_Command::mainEditServer,
     catWallet,
     "edit a server contract's label, as it appears in your wallet."},
    {"encode", OT_Command::mainEncode,
     catAdmin, "OT-base64-encode into armor."},
    {"encrypt", OT_Command::mainEncrypt,
     catAdmin,  "encrypt plaintext to a nym's public key."},
    {"exchangebasket", OT_Command::mainExchangeBasket,
     catBaskets,       "exchange in/out of a basket currency."},
    {"exportcash",   OT_Command::mainExportCash,
     catInstruments, "export a cash purse."},
    {"exportnym", OT_Command::mainExportNym,
     catWallet,   "export an OT Nym as a single importable file."},
    {"getcontract", OT_Command::mainGetContract,
     catAdmin,      "download an asset or server contract by its ID."},
    {"getmarkets", OT_Command::mainGetMarkets,
     catMarkets,   "download the list of markets."},
    {"getmyoffers", OT_Command::mainGetMyOffers,
     catMarkets,    "download mynym's list of market offers."},
    {"getoffers", OT_Command::mainGetOffers,
     catMarkets,  "download the list of market offers."},
    {"getreceipt", OT_Command::mainGetReceipt,
     catAdmin,     "downloads a box receipt based on transaction ID."},
    {"importcash",   OT_Command::mainImportCash,
     catInstruments, "import a cash purse."},
    {"importnym", OT_Command::mainImportNym,
     catWallet,   "import an OT Nym that was previously exported."},
    {"inbox",     OT_Command::mainInbox,
     catAccounts, "show inbox of a particular account."},
    {"inmail",      OT_Command::mainInmail,
     catOtherUsers, "show in-mail for a particular nym."},
    {"inpayments",  OT_Command::mainInpayments,
     catOtherUsers, "show contents of incoming payments box."},
    {"issueasset", OT_Command::mainIssueAsset,
     catAdmin,     "issue a currency contract onto an OT server."},
    {"killoffer", OT_Command::mainKillOffer,
     catMarkets,  "kill an active recurring market offer."},
    {"killplan",     OT_Command::mainKillPlan,
     catInstruments, "kill an active recurring payment plan."},
    {"newaccount", OT_Command::mainNewAccount,
     catAccounts,  "create a new asset account."},
    {"newasset", OT_Command::mainNewAsset,
     catAdmin,   "create a new asset contract."},
    {"newbasket", OT_Command::mainNewBasket,
     catBaskets,  "create a new basket currency."},
    {"newcredential", OT_Command::mainNewCredential,
     catNyms,         "create a new credential for a specific nym."},
    {"newkey", OT_Command::mainNewKey, catAdmin, "create a new symmetric key."},
    {"newnym", OT_Command::mainNewNym, catNyms, "create a new nym."},
    {"newoffer", OT_Command::mainNewOffer,
     catMarkets, "create a new market offer."},
    {"newserver", OT_Command::mainNewServer,
     catAdmin,    "create a new server contract."},
    {"outbox",    OT_Command::mainOutbox,
     catAccounts, "show outbox of a particular account."},
    {"outmail",     OT_Command::mainOutmail,
     catOtherUsers, "show out-mail for a particular nym."},
    {"outpayment",  OT_Command::mainOutpayment,
     catOtherUsers, "show contents of outgoing payments box."},
    {"passworddecrypt", OT_Command::mainPasswordDecrypt,
     catAdmin,          "password-decrypt a ciphertext using a symmetric key."},
    {"passwordencrypt", OT_Command::mainPasswordEncrypt,
     catAdmin,          "password-encrypt a plaintext using a symmetric key."},
    {"paydividend",
     OT_Command::mainPayDividend,
     catMarkets,
     "dividend payout, sent to all shareholders (in voucher form.)"},
    {"payinvoice",  OT_Command::mainPayInvoice,
     catOtherUsers, "pay an invoice."},
    {"proposeplan",  OT_Command::mainProposePlan,
     catInstruments, "as merchant, propose a payment plan to a customer."},
    {"refresh", OT_Command::mainRefresh,
     catWallet, "performs both refreshnym and refreshaccount."},
    {"refreshaccount", OT_Command::mainRefreshAccount,
     catAccounts,      "download latest intermediary files for myacct."},
    {"refreshnym", OT_Command::mainRefreshNym,
     catNyms,      "download latest intermediary files for mynym."},
    {"registernym", OT_Command::mainRegisterNym,
     catAdmin,      "register a nym onto an OT server."},
    {"revokecredential", OT_Command::mainRevokeCredential,
     catNyms,            "revoke one of a nym's credentials."},
    {"sendcash",
     OT_Command::mainSendCash,
     catOtherUsers,
     "send cash from mypurse to recipient, withdraw if necessary."},
    {"sendcheque",  OT_Command::mainSendCheque,
     catOtherUsers, "write a cheque and then send it to the recipient."},
    {"sendinvoice", OT_Command::mainSendInvoice,
     catOtherUsers, "write an invoice and then send it to the recipient."},
    {"sendmessage", OT_Command::mainSendMessage,
     catOtherUsers, "send a message to another nym's in-mail."},
    {"sendvoucher", OT_Command::mainSendVoucher,
     catOtherUsers, "withdraw a voucher and then send it to the recipient."},
    {"showaccount", OT_Command::mainShowAccount,
     catAccounts,   "show account stats for a single account."},
    {"showaccounts", OT_Command::mainShowAccounts,
     catWallet,      "show the asset accounts in the wallet."},
    {"showactive",
     OT_Command::mainShowActive,
     catInstruments,
     "show the active cron item IDs, or the details of one by ID."},
    {"showassets", OT_Command::mainShowAssets,
     catWallet,    "show the currency contracts in the wallet."},
    {"showbalance", OT_Command::mainShowBalance,
     catAccounts,   "show balance for a specific account."},
    {"showbasket", OT_Command::mainShowBasket,
     catBaskets,   "show basket currencies available in the wallet."},
    {"showcredential", OT_Command::mainShowCredential,
     catNyms,          "show a specific credential in detail."},
    {"showcredentials", OT_Command::mainShowCredentials,
     catNyms,           "show the credentials for a specific nym."},
    {"showexpired", OT_Command::mainShowExpired,
     catMisc,       "show contents of expired record box."},
    {"showincoming",
     OT_Command::mainShowIncoming,
     catWallet,
     "show incoming payments for mynym+server and/or inbox for myacct."},
    {"showmarkets", OT_Command::mainShowMarkets,
     catMarkets,    "show the list of markets."},
    {"showmint",
     OT_Command::mainShowMint,
     catAdmin,
     "show a mint file for specific asset ID. Download if necessary."},
    {"showmyoffers", OT_Command::mainShowMyOffers,
     catMarkets,     "show mynym's offers on a particular server and market."},
    {"shownym", OT_Command::mainShowNym,
     catNyms,   "show the statistics for a specific nym."},
    {"shownyms", OT_Command::mainShowNyms,
     catWallet,  "show the nyms in the wallet."},
    {"showoffers", OT_Command::mainShowOffers,
     catMarkets,   "show all offers on a particular server and market."},
    {"showoutgoing",
     OT_Command::mainShowOutgoing,
     catWallet,
     "show outgoing payments for mynym and/or outbox for myacct."},
    {"showpayment",
     OT_Command::mainShowPayment,
     catOtherUsers,
     "show the details of an incoming payment in the payments inbox."},
    {"showpurse", OT_Command::mainShowPurse,
     catWallet,   "show contents of cash purse."},
    {"showrecords", OT_Command::mainShowRecords,
     catMisc,       "show contents of record box."},
    {"showservers", OT_Command::mainShowServers,
     catWallet,     "show the server contracts in the wallet."},
    {"showwallet", OT_Command::mainShowWallet,
     catWallet,    "show wallet contents."},
    {"signcontract", OT_Command::mainSignContract,
     catAdmin,       "sign a contract, releasing all other signatures first."},
    {"transfer",  OT_Command::mainTransfer,
     catAccounts, "send a transfer from myacct to hisacct."},
    {"triggerclause", OT_Command::mainTriggerClause,
     catInstruments,  "trigger a clause on a running smart contract."},
    {"verifyreceipt",
     OT_Command::mainVerifyReceipt,
     catAccounts,
     "verify your intermediary files against the last signed receipt."},
    {"verifysignature", OT_Command::mainVerifySignature,
     catAdmin,          "verify a signature on a contract."},
    {"withdraw",     OT_Command::mainWithdrawCash,
     catInstruments, "withdraw cash. (From acct on server into local purse.)"},
    {"withdrawvoucher",
     OT_Command::mainWithdrawVoucher,
     catInstruments,
     "withdraw from myacct as a voucher (cashier's cheque.)"},
    {"writecheque",  OT_Command::mainWriteCheque,
     catInstruments, "write a cheque and print it out to the screen."},
    {"writeinvoice", OT_Command::mainWriteInvoice,
     catInstruments, "write an invoice and print it out to the screen."},
    {"", NULL, catError, ""}};

int32_t OT_OPENTXS_OT interpret_command(const string& strInput)
{
    if ("exit" == strInput || "quit" == strInput) {
        return (-2);
    }

    string spaces = "              ";

    if ("list" == strInput) {
        OTAPI_Wrap::Output(0, "\nCommands: \n\n");
        for (int32_t i = 0; map_functions[i].command != ""; i++) {
            MapFunction& func = map_functions[i];
            OTAPI_Wrap::Output(0, (func.command + spaces).substr(0, 18));
            if (i % 4 == 3) {
                OTAPI_Wrap::Output(0, "\n");
            }
        }
        OTAPI_Wrap::Output(0, "\n");

        return 0;
    }

    if ("help" == strInput) {
        string strCategory[catLast];
        for (int i = 0; i < catLast; i++) {
            strCategory[i] = "\n " + categoryName[i] + ":\n";
        }

        OTAPI_Wrap::Output(0, "\nCommands: \n\n");
        for (int32_t i = 0; map_functions[i].function != NULL; i++) {
            MapFunction& func = map_functions[i];

            string line =
                (func.command + spaces).substr(0, 18) + func.helpText + "\n";
            strCategory[func.category] += line;
        }

        for (int i = 0; i < catLast; i++) {
            OTAPI_Wrap::Output(0, strCategory[i]);
        }

        return 0; // "success" from UNIX command line perspective.;
    }

    // all other commands.
    for (int32_t i = 0; map_functions[i].command != ""; i++) {
        MapFunction& func = map_functions[i];
        if (func.command == strInput) {
            if (func.function != NULL) {
                int32_t nReturn = (*func.function)();
                OTAPI_Wrap::Output(2, "\n Returning error code: \"" +
                                          to_string(nReturn) + "\".\n\n");
                switch (nReturn) {
                case 0:
                    // This means "no error, but also didn't do anything."
                    // This is considered a form of success, so we
                    // return 0 (UNIX CLI success.)
                    //
                    return 0;
                case 1:
                    // This is an explicit success, so we
                    // return 0 (UNIX CLI success.)
                    return 0;
                case -1:
                    // This is an explicit failure, so we
                    // return -1 (UNIX failures tend to be non-zero values.)
                    return -1;
                default:
                    OTAPI_Wrap::Output(0, "\n Undefined error code: \"" +
                                              to_string(nReturn) + "\".\n\n");
                    return -1;
                }
            }
            break;
        }
    }

    OTAPI_Wrap::Output(0, "\n Undefined command: \"" + strInput +
                              "\" -- Try 'list'.\n\n");

    return -1;
}

void OT_OPENTXS_OT OT_ME::opentxs_copy_variables()
{
    OTVariable* pVar;
    Args = (pVar = FindVariable("Args")) == NULL ? "" : pVar->GetValueString();
    HisAcct =
        (pVar = FindVariable("HisAcct")) == NULL ? "" : pVar->GetValueString();
    HisNym =
        (pVar = FindVariable("HisNym")) == NULL ? "" : pVar->GetValueString();
    HisPurse =
        (pVar = FindVariable("HisPurse")) == NULL ? "" : pVar->GetValueString();
    MyAcct =
        (pVar = FindVariable("MyAcct")) == NULL ? "" : pVar->GetValueString();
    MyNym =
        (pVar = FindVariable("MyNym")) == NULL ? "" : pVar->GetValueString();
    MyPurse =
        (pVar = FindVariable("MyPurse")) == NULL ? "" : pVar->GetValueString();
    Server =
        (pVar = FindVariable("Server")) == NULL ? "" : pVar->GetValueString();
}

int OT_OPENTXS_OT OT_ME::opentxs_main_loop()
{
    // See if the command was passed in on the command line.
    if (VerifyExists("Args", false)) {
        OTAPI_Wrap::Output(1, "\nCommand: " + Args + "\n\n");
        string strCommand = OT_CLI_GetValueByKey(Args, "ot_cli_command");

        if (VerifyStringVal(strCommand)) // command was passed in on the command
                                         // line...
        {
            return interpret_command(strCommand);
        }
    }

    // Otherwise, show list.
    return interpret_command("list");
}

} // namespace opentxs
