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
    catAdmin = 1,
    catWallet = 2,
    catMisc = 3,
    catMarkets = 4,
    catAccounts = 5,
    catOtherUsers = 6,
    catInstruments = 7,
    catBaskets = 8,
    catNyms = 9,
} Category;

static string categoryName[] = {
    "Category Error",           "Advanced utilities",    "The user wallet",
    "Misc",                     "Markets (bid/ask)",     "Asset accounts",
    "Dealing with other users", "Financial instruments", "Basket currencies",
    "Pseudonyms"};

typedef struct
{
    string command;
    int32_t category;
} MapCategory;

MapCategory map_categories[] = {{"acceptall", catAccounts},
                                {"acceptinbox", catAccounts},
                                {"acceptinvoices", catAccounts},
                                {"acceptmoney", catAccounts},
                                {"acceptpayments", catAccounts},
                                {"acceptreceipts", catAccounts},
                                {"accepttransfers", catAccounts},
                                {"addasset", catWallet},
                                {"addserver", catWallet},
                                {"addsignature", catAdmin},
                                {"cancel", catInstruments},
                                {"changepw", catWallet},
                                {"checknym", catOtherUsers},
                                {"clearexpired", catMisc},
                                {"clearrecords", catMisc},
                                {"confirm", catInstruments},
                                {"decode", catAdmin},
                                {"decrypt", catAdmin},
                                {"deleteinmail", catOtherUsers},
                                {"deleteoutmail", catOtherUsers},
                                {"deposit", catAccounts},
                                {"discard", catInstruments},
                                {"editaccount", catWallet},
                                {"editasset", catWallet},
                                {"editnym", catWallet},
                                {"editserver", catWallet},
                                {"encode", catAdmin},
                                {"encrypt", catAdmin},
                                {"exchangebasket", catBaskets},
                                {"exportcash", catInstruments},
                                {"exportnym", catWallet},
                                {"getcontract", catAdmin},
                                {"getmarkets", catMarkets},
                                {"getmyoffers", catMarkets},
                                {"getoffers", catMarkets},
                                {"getreceipt", catAdmin},
                                {"importcash", catInstruments},
                                {"importnym", catWallet},
                                {"inbox", catAccounts},
                                {"inmail", catOtherUsers},
                                {"inpayments", catOtherUsers},
                                {"issueasset", catAdmin},
                                {"killoffer", catMarkets},
                                {"killplan", catInstruments},
                                {"newaccount", catAccounts},
                                {"newasset", catAdmin},
                                {"newbasket", catBaskets},
                                {"newcredential", catNyms},
                                {"newkey", catAdmin},
                                {"newnym", catNyms},
                                {"newoffer", catMarkets},
                                {"newserver", catAdmin},
                                {"outbox", catAccounts},
                                {"outmail", catOtherUsers},
                                {"outpayment", catOtherUsers},
                                {"passworddecrypt", catAdmin},
                                {"passwordencrypt", catAdmin},
                                {"paydividend", catMarkets},
                                {"payinvoice", catOtherUsers},
                                {"proposeplan", catInstruments},
                                {"refresh", catWallet},
                                {"refreshaccount", catAccounts},
                                {"refreshnym", catNyms},
                                {"registernym", catAdmin},
                                {"revokecredential", catNyms},
                                {"sendcash", catOtherUsers},
                                {"sendcheque", catOtherUsers},
                                {"sendinvoice", catOtherUsers},
                                {"sendmessage", catOtherUsers},
                                {"sendvoucher", catOtherUsers},
                                {"showaccount", catAccounts},
                                {"showaccounts", catWallet},
                                {"showactive", catInstruments},
                                {"showassets", catWallet},
                                {"showbalance", catAccounts},
                                {"showbasket", catBaskets},
                                {"showcredential", catNyms},
                                {"showcredentials", catNyms},
                                {"showexpired", catMisc},
                                {"showincoming", catWallet},
                                {"showmarkets", catMarkets},
                                {"showmint", catAdmin},
                                {"showmyoffers", catMarkets},
                                {"shownym", catNyms},
                                {"shownyms", catWallet},
                                {"showoffers", catMarkets},
                                {"showoutgoing", catWallet},
                                {"showpayment", catOtherUsers},
                                {"showpurse", catWallet},
                                {"showrecords", catMisc},
                                {"showservers", catWallet},
                                {"showwallet", catWallet},
                                {"signcontract", catAdmin},
                                {"transfer", catAccounts},
                                {"triggerclause", catInstruments},
                                {"verifyreceipt", catAccounts},
                                {"verifysignature", catAdmin},
                                {"withdraw", catInstruments},
                                {"withdrawvoucher", catInstruments},
                                {"writecheque", catInstruments},
                                {"writeinvoice", catInstruments},
                                {"", 0}};

typedef struct
{
    string command;
    string helpText;
} MapHelp;

MapHelp map_help[] = {
    {"acceptall",
     "accept all incoming transfers, receipts, payments, and invoices."},
    {"acceptinbox",
     "accept all incoming transfers and receipts in MyAcct's inbox."},
    {"acceptinvoices", "pay all invoices in MyNym's payments inbox."},
    {"acceptmoney", "accept all incoming transfers and payments into MyAcct."},
    {"acceptpayments",
     "accept all incoming payments in MyNym's payments inbox."},
    {"acceptreceipts", "accept all receipts in MyAcct's inbox."},
    {"accepttransfers", "accept all incoming transfers in MyAcct's inbox."},
    {"addasset",
     "paste an existing asset contract, import it into your wallet."},
    {"addserver",
     "paste an existing server contract, import it into your wallet."},
    {"addsignature", "add a signature to a contract without releasing others."},
    {"cancel",
     "cancel a not-yet-cashed, outgoing instrument from outpayment box."},
    {"changepw", "change the master passphrase for the wallet."},
    {"checknym", "download a nym's public key based on his ID."},
    {"clearexpired", "clear all expired records."},
    {"clearrecords", "clear all archived records and receipts."},
    {"confirm", "confirm your agreement to a smart contract or payment plan."},
    {"decode", "OT-base64-decode out of armor."},
    {"decrypt", "decrypt ciphertext using nym's private key."},
    {"deleteinmail", "delete an in-mail item."},
    {"deleteoutmail", "delete an out-mail item."},
    {"deposit", "deposit cash, cheque, voucher, or tokens."},
    {"discard",
     "discard a not-yet-cashed, incoming instrument from payments inbox."},
    {"editaccount",
     "edit an asset account's label, as it appears in your wallet."},
    {"editasset",
     "edit a currency contract's label, as it appears in your wallet."},
    {"editnym", "edit the nym's label, as it appears in your wallet."},
    {"editserver",
     "edit a server contract's label, as it appears in your wallet."},
    {"encode", "OT-base64-encode into armor."},
    {"encrypt", "encrypt plaintext to a nym's public key."},
    {"exchangebasket", "exchange in/out of a basket currency."},
    {"exportcash", "export a cash purse."},
    {"exportnym", "export an OT Nym as a single importable file."},
    {"getcontract", "download an asset or server contract by its ID."},
    {"getmarkets", "download the list of markets."},
    {"getmyoffers", "download mynym's list of market offers."},
    {"getoffers", "download the list of market offers."},
    {"getreceipt", "downloads a box receipt based on transaction ID."},
    {"importcash", "import a cash purse."},
    {"importnym", "import an OT Nym that was previously exported."},
    {"inbox", "show inbox of a particular account."},
    {"inmail", "show in-mail for a particular nym."},
    {"inpayments", "show contents of incoming payments box."},
    {"issueasset", "issue a currency contract onto an OT server."},
    {"killoffer", "kill an active, still-running, recurring market offer."},
    {"killplan", "kill an active, still-running, recurring payment plan."},
    {"newaccount", "create a new asset account."},
    {"newasset", "create a new asset contract."},
    {"newbasket", "create a new basket currency."},
    {"newcredential", "create a new credential for a specific nym."},
    {"newkey", "create a new symmetric key."}, {"newnym", "create a new nym."},
    {"newoffer", "create a new market offer."},
    {"newserver", "create a new server contract."},
    {"outbox", "show outbox of a particular account."},
    {"outmail", "show out-mail for a particular nym."},
    {"outpayment", "show contents of outgoing payments box."},
    {"passworddecrypt", "password-decrypt a ciphertext using a symmetric key."},
    {"passwordencrypt", "password-encrypt a plaintext using a symmetric key."},
    {"paydividend",
     "dividend payout, sent to all shareholders (in voucher form.)"},
    {"payinvoice", "pay an invoice."},
    {"proposeplan", "as merchant, propose a payment plan to a customer."},
    {"refresh", "performs both refreshnym and refreshaccount."},
    {"refreshaccount", "download latest intermediary files for myacct."},
    {"refreshnym", "download latest intermediary files for mynym."},
    {"registernym", "register a nym onto an OT server."},
    {"revokecredential", "revoke one of a nym's credentials."},
    {"sendcash", "send cash from mypurse to recipient, withdraw if necessary."},
    {"sendcheque", "write a cheque and then send it to the recipient."},
    {"sendinvoice", "write an invoice and then send it to the recipient."},
    {"sendmessage", "send a message to another nym's in-mail."},
    {"sendvoucher", "withdraw a voucher and then send it to the recipient."},
    {"showaccount", "show account stats for a single account."},
    {"showaccounts", "show the asset accounts in the wallet."},
    {"showactive",
     "show the active cron item IDs, or the details of one by ID."},
    {"showassets", "show the currency contracts in the wallet."},
    {"showbalance", "show balance for a specific account."},
    {"showbasket", "show basket currencies available in the wallet."},
    {"showcredential", "show a specific credential in detail."},
    {"showcredentials", "show the credentials for a specific nym."},
    {"showexpired", "show contents of expired record box."},
    {"showincoming",
     "show incoming payments for mynym+server and/or inbox for myacct."},
    {"showmarkets", "show the list of markets."},
    {"showmint",
     "show a mint file for specific asset ID. Download if necessary."},
    {"showmyoffers", "show mynym's offers on a particular server and market."},
    {"shownym", "show the statistics for a specific nym."},
    {"shownyms", "show the nyms in the wallet."},
    {"showoffers", "show all offers on a particular server and market."},
    {"showoutgoing",
     "show outgoing payments for mynym and/or outbox for myacct."},
    {"showpayment",
     "show the details of an incoming payment in the payments inbox."},
    {"showpurse", "show contents of cash purse."},
    {"showrecords", "show contents of record box."},
    {"showservers", "show the server contracts in the wallet."},
    {"showwallet", "show wallet contents."},
    {"signcontract", "sign a contract, releasing all other signatures first."},
    {"transfer", "send a transfer from myacct to hisacct."},
    {"triggerclause", "trigger a clause on a running smart contract."},
    {"verifyreceipt",
     "verify your intermediary files against the last signed receipt."},
    {"verifysignature", "verify a signature on a contract."},
    {"withdraw", "withdraw cash. (From acct on server into local purse.)"},
    {"withdrawvoucher",
     "withdraw from myacct as a voucher (cashier's cheque.)"},
    {"writecheque", "write a cheque and print it out to the screen."},
    {"writeinvoice", "write an invoice and print it out to the screen."},
    {"", ""}};

typedef struct
{
    string command;
    int32_t (*function)();
} MapFunction;

MapFunction map_functions[] = {
    {"acceptall", OT_Command::mainAcceptAll},     // accept all incoming
                                                  // transfers, receipts,
                                                  // payments, and invoices.
    {"acceptinbox", OT_Command::mainAcceptInbox}, // accept all incoming
                                                  // transfers and receipts in
                                                  // MyAcct's inbox.
    {"acceptinvoices", OT_Command::mainAcceptInvoices}, // pay all invoices in
                                                        // MyNym's payments
                                                        // inbox.
    {"acceptmoney", OT_Command::mainAcceptMoney},       // accept all incoming
    // transfers and payments
    // into MyAcct.
    {"acceptpayments", OT_Command::mainAcceptPayments}, // accept all incoming
                                                        // payments in MyNym's
                                                        // payments inbox.
    {"acceptreceipts", OT_Command::mainAcceptReceipts}, // accept all receipts
                                                        // in MyAcct's inbox.
    {"accepttransfers",
     OT_Command::mainAcceptTransfers},      // accept all incoming transfers in
                                            // MyAcct's inbox.
    {"addasset", OT_Command::mainAddAsset}, // paste an existing asset
                                            // contract, import it into your
                                            // wallet.
    {"addserver", OT_Command::mainAddServer}, // paste an existing server
                                              // contract, import it into your
                                              // wallet.
    {"addsignature", OT_Command::mainAddSignature}, // add a signature to a
                                                    // contract without
                                                    // releasing others.
    {"cancel", OT_Command::mainCancel},             // cancel a not-yet-cashed,
                                                    // outgoing instrument from
                                                    // outpayment box.
    {"changepw", OT_Command::mainChangePw},         // Change the master
                                                    // passphrase for the
                                                    // wallet.
    {"checknym", OT_Command::mainCheckNym}, // download a nym's public key
                                            // based on his ID.
    {"clearexpired",
     OT_Command::mainClearExpired}, // clear all expired records.
    {"clearrecords", OT_Command::mainClearRecords}, // clear all archived
                                                    // records and receipts.
    {"confirm", OT_Command::mainConfirm}, // confirm your agreement to a smart
                                          // contract or payment plan.
    {"decode", OT_Command::mainDecode},   // OT-base64-decode out of armor.
    {"decrypt",
     OT_Command::mainDecrypt}, // decrypt ciphertext using nym's private key.
    {"deleteinmail", OT_Command::mainDeleteInmail},   // delete an in-mail item
    {"deleteoutmail", OT_Command::mainDeleteOutmail}, // delete an out-mail item
    {"deposit", OT_Command::mainDeposit},         // deposit cash purse, cheque,
                                                  // voucher, or invoice.
    {"discard", OT_Command::mainDiscard},         // discard a not-yet-cashed,
                                                  // incoming instrument from
                                                  // payments inbox.
    {"editaccount", OT_Command::mainEditAccount}, // edit an asset account's
    // label, as it appears in your
    // wallet.
    {"editasset", OT_Command::mainEditAsset},   // edit a currency contract's
                                                // label, as it appears in your
                                                // wallet.
    {"editnym", OT_Command::mainEditNym},       // edit the nym's label, as it
                                                // appears in your wallet.
    {"editserver", OT_Command::mainEditServer}, // edit a server contract's
                                                // label, as it appears in
                                                // your wallet.
    {"encode", OT_Command::mainEncode},         // OT-base64-encode into armor.
    {"encrypt",
     OT_Command::mainEncrypt}, // encrypt plaintext to a nym's public key.
    {"exchangebasket",
     OT_Command::mainExchangeBasket}, // exchange in/out of a basket currency.
    {"exportcash", OT_Command::mainExportCash}, // Export a cash purse.
    {"exportnym", OT_Command::mainExportNym},   // Export an OT Nym as a single
                                                // importable file.
    {"getcontract", OT_Command::mainGetContract}, // download an asset or
                                                  // server contract by
                                                  // its ID.
    {"getmarkets", OT_Command::mainGetMarkets}, // download the list of markets.
    {"getmyoffers", OT_Command::mainGetMyOffers}, // download the list of market
                                                  // offers placed by mynym.
    {"getoffers",
     OT_Command::mainGetOffers}, // download the list of market offers.
    {"getreceipt", OT_Command::mainGetReceipt}, // downloads a box
                                                // receipt based on
                                                // transaction ID.
    {"importcash", OT_Command::mainImportCash}, // Import a cash purse.
    {"importnym", OT_Command::mainImportNym},   // Import an OT Nym that was
                                                // previously exported.
    {"inbox", OT_Command::mainInbox},   // show inbox of a particular account.
    {"inmail", OT_Command::mainInmail}, // show in-mail for a particular nym.
    {"inpayments", OT_Command::mainInpayments}, // show contents of
    // incoming payments
    // box.
    {"issueasset", OT_Command::mainIssueAsset}, // issue a currency contract
                                                // onto an OT server.
    {"killoffer", OT_Command::mainKillOffer},   // kill a still-running,
                                                // recurring market offer.
    {"killplan", OT_Command::mainKillPlan}, // kill a still-running, recurring
                                            // payment plan.
    {"newaccount", OT_Command::mainNewAccount}, // create a new asset account.
    {"newasset", OT_Command::mainNewAsset},     // create a new asset contract.
    {"newbasket", OT_Command::mainNewBasket},   // create a new basket currency.
    {"newcredential",
     OT_Command::mainNewCredential},          // create a new credential for
                                              // a specific nym.
    {"newkey", OT_Command::mainNewKey},       // create a new symmetric key.
    {"newnym", OT_Command::mainNewNym},       // create a new nym.
    {"newoffer", OT_Command::mainNewOffer},   // create a new market offer.
    {"newserver", OT_Command::mainNewServer}, // create a new server contract.
    {"outbox", OT_Command::mainOutbox}, // show outbox of a particular account.
    {"outmail", OT_Command::mainOutmail}, // show out-mail for a particular nym.
    {"outpayment", OT_Command::mainOutpayment}, // show contents of
                                                // outgoing payments box.
    {"passworddecrypt", OT_Command::mainPasswordDecrypt}, // password-decrypt a
                                                          // ciphertext using a
                                                          // symmetric key.
    {"passwordencrypt", OT_Command::mainPasswordEncrypt}, // password-encrypt a
                                                          // plaintext using a
                                                          // symmetric key.
    {"paydividend", OT_Command::mainPayDividend}, // dividend payout, sent to
                                                  // all shareholders in
                                                  // voucher form.
    {"payinvoice", OT_Command::mainPayInvoice},   // pay an invoice.
    {"proposeplan", OT_Command::mainProposePlan}, // as merchant, propose a
                                                  // payment plan to a customer.
    {"refresh", OT_Command::mainRefresh}, // "performs both refreshnym and
                                          // refreshaccount."
    {"refreshaccount", OT_Command::mainRefreshAccount}, // "download latest
    // intermediary files for
    // myacct."
    {"refreshnym", OT_Command::mainRefreshNym}, // download latest
                                                // intermediary files for
                                                // mynym.
    {"registernym",
     OT_Command::mainRegisterNym}, // register a nym onto an OT server.
    {"revokecredential",
     OT_Command::mainRevokeCredential}, // revoke one of a nym's credentials.
    {"sendcash", OT_Command::mainSendCash},     // send cash from mypurse to
                                                // recipient, withdraw if
                                                // necessary.
    {"sendcheque", OT_Command::mainSendCheque}, // write a cheque and then send
                                                // it to the recipient.
    {"sendinvoice", OT_Command::mainSendInvoice}, // write an invoice and then
                                                  // send it to the recipient.
    {"sendmessage",
     OT_Command::mainSendMessage}, // send a message to another nym's in-mail.
    {"sendvoucher", OT_Command::mainSendVoucher}, // withdraw a voucher and
                                                  // then send it to the
                                                  // recipient.
    {"showaccount",
     OT_Command::mainShowAccount}, // show account stats for a single account.
    {"showaccounts",
     OT_Command::mainShowAccounts}, // show the asset accounts in the wallet.
    {"showactive", OT_Command::mainShowActive}, // show the active cron item
                                                // IDs, or the details of one
                                                // by ID.
    {"showassets", OT_Command::mainShowAssets}, // show the currency contracts
                                                // in the wallet.
    {"showbalance",
     OT_Command::mainShowBalance}, // show balance for a specific account.
    {"showbasket", OT_Command::mainShowBasket}, // show basket currencies
                                                // available in the wallet.
    {"showcredential",
     OT_Command::mainShowCredential}, // show a specific credential in detail.
    {"showcredentials",
     OT_Command::mainShowCredentials}, // show the credentials
                                       // for a specific nym.
    {"showexpired",
     OT_Command::mainShowExpired}, // show contents of expired box.
    {"showincoming", OT_Command::mainShowIncoming}, // show incoming payments
                                                    // for mynym+server and/or
                                                    // inbox for myacct.
    {"showmarkets", OT_Command::mainShowMarkets},   // show the list of markets.
    {"showmint", OT_Command::mainShowMint}, // show a mint file for specific
                                            // asset ID. Download if
                                            // necessary.
    {"showmyoffers", OT_Command::mainShowMyOffers}, // show mynym's offers
                                                    // on a particular
                                                    // server and market.
    {"shownym",
     OT_Command::mainShowNym}, // show the statistics for a specific nym.
    {"shownyms", OT_Command::mainShowNyms},     // show the nyms in the wallet.
    {"showoffers", OT_Command::mainShowOffers}, // show all offers on a
                                                // particular server
                                                // and market.
    {"showoutgoing", OT_Command::mainShowOutgoing}, // show outgoing payments
                                                    // for mynym and/or outbox
                                                    // for myacct.
    {"showpayment", OT_Command::mainShowPayment},   // show the details of a
                                                    // payment in the payments
                                                    // inbox.
    {"showpurse", OT_Command::mainShowPurse}, // show contents of cash purse.
    {"showrecords",
     OT_Command::mainShowRecords}, // show contents of record box.
    {"showservers",
     OT_Command::mainShowServers}, // show the server contracts in the wallet.
    {"showwallet", OT_Command::mainShowWallet}, // show wallet contents.
    {"signcontract",
     OT_Command::mainSignContract}, // sign a contract, releasing all
                                    // other signatures first.
    {"transfer",
     OT_Command::mainTransfer}, // send a transfer from myacct to hisacct.
    {"triggerclause", OT_Command::mainTriggerClause}, // trigger a clause on a
                                                      // running smart contract.
    {"verifyreceipt",
     OT_Command::mainVerifyReceipt}, // verify the intermediary files
                                     // against the last signed receipt.
    {"verifysignature",
     OT_Command::mainVerifySignature}, // verify a signature on a contract.
    {"withdraw", OT_Command::mainWithdrawCash}, // withdraw cash (from acct on
                                                // server to local purse.)
    {"withdrawvoucher", OT_Command::mainWithdrawVoucher}, // withdraw a voucher
    // (cashier's cheque).
    {"writecheque", OT_Command::mainWriteCheque},   // write a cheque and print
                                                    // it out to the screen.
    {"writeinvoice", OT_Command::mainWriteInvoice}, // write an invoice and
                                                    // print it out to the
                                                    // screen.
    {"", NULL}};

int32_t OT_OPENTXS_OT interpret_command(const string& strInput)
{
    if ("quit" == strInput) {
        return (-2);
    }

    if ("list" == strInput) {
        OTAPI_Wrap::Output(0, "\nCommands: \n\n");
        for (int32_t i = 0; map_functions[i].command != ""; i++) {
            MapFunction& func = map_functions[i];
            OTAPI_Wrap::Output(
                0, func.command + (func.command.length() > 7 ? "\t" : "\t\t"));
            if (i % 4 == 3) {
                OTAPI_Wrap::Output(0, "\n");
            }
        }
        OTAPI_Wrap::Output(0, "\n");

        return 0; // "success" from UNIX command line perspective.;
    }

    if ("help" == strInput) {
        string strAdmin = "";
        string strWallet = "";
        string strMisc = "";
        string strMarkets = "";
        string strAccounts = "";
        string strOtherUsers = "";
        string strInstruments = "";
        string strBaskets = "";
        string strNyms = "";

        OTAPI_Wrap::Output(0, "\nCommands: \n\n");
        for (int32_t i = 0; map_functions[i].command != ""; i++) {
            MapFunction& func = map_functions[i];

            string line = "";
            for (int32_t j = 0; map_help[j].command != ""; j++) {
                MapHelp& help = map_help[j];
                if (help.command == func.command) {
                    line = help.command +
                           (help.command.length() > 7 ? "\t" : "\t\t") +
                           help.helpText + "\n";
                    break;
                }
            }

            for (int32_t j = 0; map_categories[j].command != ""; j++) {
                MapCategory& cat = map_categories[j];
                if (cat.command == func.command) {
                    switch (cat.category) {
                    case catAdmin:
                        strAdmin += line;
                        break;
                    case catWallet:
                        strWallet += line;
                        break;
                    case catMisc:
                        strMisc += line;
                        break;
                    case catMarkets:
                        strMarkets += line;
                        break;
                    case catAccounts:
                        strAccounts += line;
                        break;
                    case catOtherUsers:
                        strOtherUsers += line;
                        break;
                    case catInstruments:
                        strInstruments += line;
                        break;
                    case catBaskets:
                        strBaskets += line;
                        break;
                    case catNyms:
                        strNyms += line;
                        break;
                    }
                    break;
                }
            }
        }

        OTAPI_Wrap::Output(0,
                           "\n " + categoryName[catAdmin] + ": \\n" + strAdmin);
        OTAPI_Wrap::Output(0, "\n " + categoryName[catWallet] + ": \\n" +
                                  strWallet);
        OTAPI_Wrap::Output(0,
                           "\n " + categoryName[catMisc] + ": \\n" + strMisc);
        OTAPI_Wrap::Output(0, "\n " + categoryName[catMarkets] + ": \\n" +
                                  strMarkets);
        OTAPI_Wrap::Output(0, "\n " + categoryName[catAccounts] + ": \\n" +
                                  strAccounts);
        OTAPI_Wrap::Output(0, "\n " + categoryName[catOtherUsers] + ": \\n" +
                                  strOtherUsers);
        OTAPI_Wrap::Output(0, "\n " + categoryName[catInstruments] + ": \\n" +
                                  strInstruments);
        OTAPI_Wrap::Output(0, "\n " + categoryName[catBaskets] + ": \\n" +
                                  strBaskets);
        OTAPI_Wrap::Output(0,
                           "\n " + categoryName[catNyms] + ": \\n" + strNyms);
        OTAPI_Wrap::Output(0, "\n");

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
