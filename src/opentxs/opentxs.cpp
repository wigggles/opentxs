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

#include "opentxs.hpp"

#include "../client/commands/CmdAcceptAll.hpp"
#include "../client/commands/CmdAcceptInbox.hpp"
#include "../client/commands/CmdAcceptInvoices.hpp"
#include "../client/commands/CmdAcceptMoney.hpp"
#include "../client/commands/CmdAcceptPayments.hpp"
#include "../client/commands/CmdAcceptReceipts.hpp"
#include "../client/commands/CmdAcceptTransfers.hpp"
#include "../client/commands/CmdAddAsset.hpp"
#include "../client/commands/CmdAddServer.hpp"
#include "../client/commands/CmdAddSignature.hpp"
#include "../client/commands/CmdCancel.hpp"
#include "../client/commands/CmdChangePw.hpp"
#include "../client/commands/CmdCheckNym.hpp"
#include "../client/commands/CmdClearExpired.hpp"
#include "../client/commands/CmdClearRecords.hpp"
#include "../client/commands/CmdConfirm.hpp"
#include "../client/commands/CmdDecode.hpp"
#include "../client/commands/CmdDecrypt.hpp"
#include "../client/commands/CmdDeleteInmail.hpp"
#include "../client/commands/CmdDeleteOutmail.hpp"
#include "../client/commands/CmdDeposit.hpp"
#include "../client/commands/CmdDiscard.hpp"
#include "../client/commands/CmdEditAccount.hpp"
#include "../client/commands/CmdEditAsset.hpp"
#include "../client/commands/CmdEditNym.hpp"
#include "../client/commands/CmdEditServer.hpp"
#include "../client/commands/CmdEncode.hpp"
#include "../client/commands/CmdEncrypt.hpp"
#include "../client/commands/CmdExchangeBasket.hpp"
#include "../client/commands/CmdExportCash.hpp"
#include "../client/commands/CmdExportNym.hpp"
#include "../client/commands/CmdGetContract.hpp"
#include "../client/commands/CmdGetMarkets.hpp"
#include "../client/commands/CmdGetMyOffers.hpp"
#include "../client/commands/CmdGetOffers.hpp"
#include "../client/commands/CmdGetReceipt.hpp"
#include "../client/commands/CmdImportCash.hpp"
#include "../client/commands/CmdImportNym.hpp"
#include "../client/commands/CmdInbox.hpp"
#include "../client/commands/CmdInmail.hpp"
#include "../client/commands/CmdInpayments.hpp"
#include "../client/commands/CmdIssueAsset.hpp"
#include "../client/commands/CmdKillOffer.hpp"
#include "../client/commands/CmdKillPlan.hpp"
#include "../client/commands/CmdNewAccount.hpp"
#include "../client/commands/CmdNewAsset.hpp"
#include "../client/commands/CmdNewBasket.hpp"
#include "../client/commands/CmdNewKey.hpp"
#include "../client/commands/CmdNewNymLegacy.hpp"
#include "../client/commands/CmdNewOffer.hpp"
#include "../client/commands/CmdNewServer.hpp"
#include "../client/commands/CmdOutbox.hpp"
#include "../client/commands/CmdOutmail.hpp"
#include "../client/commands/CmdOutpayment.hpp"
#include "../client/commands/CmdPasswordDecrypt.hpp"
#include "../client/commands/CmdPasswordEncrypt.hpp"
#include "../client/commands/CmdPayDividend.hpp"
#include "../client/commands/CmdPayInvoice.hpp"
#include "../client/commands/CmdProposePlan.hpp"
#include "../client/commands/CmdRefresh.hpp"
#include "../client/commands/CmdRefreshAccount.hpp"
#include "../client/commands/CmdRefreshNym.hpp"
#include "../client/commands/CmdRegisterNym.hpp"
#include "../client/commands/CmdSendCash.hpp"
#include "../client/commands/CmdSendCheque.hpp"
#include "../client/commands/CmdSendInvoice.hpp"
#include "../client/commands/CmdSendMessage.hpp"
#include "../client/commands/CmdSendVoucher.hpp"
#include "../client/commands/CmdShowAccount.hpp"
#include "../client/commands/CmdShowAccounts.hpp"
#include "../client/commands/CmdShowActive.hpp"
#include "../client/commands/CmdShowAssets.hpp"
#include "../client/commands/CmdShowBalance.hpp"
#include "../client/commands/CmdShowBasket.hpp"
#include "../client/commands/CmdShowCredential.hpp"
#include "../client/commands/CmdShowCredentials.hpp"
#include "../client/commands/CmdShowExpired.hpp"
#include "../client/commands/CmdShowMarkets.hpp"
#include "../client/commands/CmdShowMint.hpp"
#include "../client/commands/CmdShowMyOffers.hpp"
#include "../client/commands/CmdShowNym.hpp"
#include "../client/commands/CmdShowNyms.hpp"
#include "../client/commands/CmdShowOffers.hpp"
#include "../client/commands/CmdShowPayment.hpp"
#include "../client/commands/CmdShowPurse.hpp"
#include "../client/commands/CmdShowRecords.hpp"
#include "../client/commands/CmdShowServers.hpp"
#include "../client/commands/CmdShowWallet.hpp"
#include "../client/commands/CmdSignContract.hpp"
#include "../client/commands/CmdShowWallet.hpp"
#include "../client/commands/CmdTransfer.hpp"
#include "../client/commands/CmdTriggerClause.hpp"
#include "../client/commands/CmdUsageCredits.hpp"
#include "../client/commands/CmdVerifyReceipt.hpp"
#include "../client/commands/CmdVerifySignature.hpp"
#include "../client/commands/CmdWithdrawCash.hpp"
#include "../client/commands/CmdWithdrawVoucher.hpp"
#include "../client/commands/CmdWriteCheque.hpp"
#include "../client/commands/CmdWriteInvoice.hpp"

#include <opentxs/client/OpenTransactions.hpp>
#include <opentxs/client/OTAPI.hpp>

#include <opentxs/core/Log.hpp>
#include <opentxs/core/util/OTPaths.hpp>
#include <opentxs/core/Version.hpp>

#include <opentxs/core/crypto/OTCallback.hpp>
#include <opentxs/core/crypto/OTCaller.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>

#include <anyoption/anyoption.hpp>

#include <algorithm>
#include <cctype>
#include <string>
#include <functional>

using namespace opentxs;
using namespace std;

const char* categoryName[] = {
    "Category Error",           "Advanced utilities",    "The user wallet",
    "Misc",                     "Markets (bid/ask)",     "Asset accounts",
    "Dealing with other users", "Financial instruments", "Basket currencies",
    "Pseudonyms"};

CmdBase* cmds[] = {new CmdAcceptAll,       new CmdAcceptInbox,
                   new CmdAcceptInvoices,  new CmdAcceptMoney,
                   new CmdAcceptPayments,  new CmdAcceptReceipts,
                   new CmdAcceptTransfers, new CmdAddAsset,
                   new CmdAddServer,       new CmdAddSignature,
                   new CmdCancel,          new CmdChangePw,
                   new CmdCheckNym,        new CmdClearExpired,
                   new CmdClearRecords,    new CmdConfirm,
                   new CmdDecode,          new CmdDecrypt,
                   new CmdDeleteInmail,    new CmdDeleteOutmail,
                   new CmdDeposit,         new CmdDiscard,
                   new CmdEditAccount,     new CmdEditAsset,
                   new CmdEditNym,         new CmdEditServer,
                   new CmdEncode,          new CmdEncrypt,
                   new CmdExchangeBasket,  new CmdExportCash,
                   new CmdExportNym,       new CmdGetInstrumentDefinition,
                   new CmdGetMarkets,      new CmdGetMyOffers,
                   new CmdGetOffers,       new CmdGetReceipt,
                   new CmdImportCash,      new CmdImportNym,
                   new CmdInbox,           new CmdInmail,
                   new CmdInpayments,      new CmdIssueAsset,
                   new CmdKillOffer,       new CmdKillPlan,
                   new CmdNewAccount,      new CmdNewAsset,
                   new CmdNewKey,          new CmdNewNymLegacy,
                   new CmdNewOffer,        new CmdNewServer,
                   new CmdOutbox,          new CmdOutmail,
                   new CmdNewBasket,       new CmdOutpayment,
                   new CmdPasswordDecrypt, new CmdPasswordEncrypt,
                   new CmdPayDividend,     new CmdPayInvoice,
                   new CmdProposePlan,     new CmdRefresh,
                   new CmdRefreshAccount,  new CmdRefreshNym,
                   new CmdRegisterNym,     new CmdSendCash,
                   new CmdSendCheque,      new CmdSendInvoice,
                   new CmdSendMessage,     new CmdSendVoucher,
                   new CmdShowAccount,     new CmdShowAccounts,
                   new CmdShowActive,      new CmdShowAssets,
                   new CmdShowBalance,     new CmdShowBasket,
                   new CmdShowCredential,  new CmdShowCredentials,
                   new CmdShowExpired,     new CmdShowMarkets,
                   new CmdShowMint,        new CmdShowMyOffers,
                   new CmdShowNym,         new CmdShowNyms,
                   new CmdShowOffers,      new CmdShowPayment,
                   new CmdShowPurse,       new CmdShowRecords,
                   new CmdShowServers,     new CmdShowWallet,
                   new CmdSignContract,    new CmdTransfer,
                   new CmdTriggerClause,   new CmdUsageCredits,
                   new CmdVerifyReceipt,   new CmdVerifySignature,
                   new CmdWithdrawCash,    new CmdWithdrawVoucher,
                   new CmdWriteCheque,     new CmdWriteInvoice,
                   nullptr};

Opentxs::Opentxs()
{
    OTAPI_Wrap::AppInit();
}

Opentxs::~Opentxs()
{
    OTAPI_Wrap::AppCleanup();
}

string& Opentxs::ltrim(string& s)
{
    s.erase(s.begin(),
            find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
    return s;
}

string& Opentxs::rtrim(string& s)
{
    s.erase(
        find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(),
        s.end());
    return s;
}

string& Opentxs::trim(string& s)
{
    return ltrim(rtrim(s));
}

const char* Opentxs::getOption(AnyOption& opt, const char* optionName,
                               const char* defaultName)
{
    // can we get the default value from the command line?
    const char* value = opt.getValue(optionName);
    if (value != nullptr) {
        otWarn << "Option  " << optionName << ": " << value << "\n";
        return value;
    }

    // can we get the default value from the options file?
    if (nullptr != defaultName) {
        value = opt.getValue(defaultName);
        if (value != nullptr) {
            otWarn << "Default " << optionName << ": " << value << "\n";
            return value;
        }
    }

    // clear option value
    return "";
}

void Opentxs::loadOptions(AnyOption& opt)
{
    String configPath(OTPaths::AppDataFolder());
    bool configPathFound = configPath.Exists() && 3 < configPath.GetLength();
    OT_ASSERT_MSG(configPathFound,
                  "RegisterAPIWithScript: Must set Config Path first!\n");
    otWarn << "Using configuration path: " << configPath << "\n";

    opt.addUsage("");
    opt.addUsage(" Opentxs CLI Usage:  ");
    opt.addUsage("");

    opt.setCommandFlag("echocommand");
    opt.setCommandFlag("echoexpand");
    opt.setCommandFlag("errorlist");
    opt.setCommandFlag("noprompt");
    opt.setCommandFlag("dummy-passphrase");
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

    String optionsFile("command-line-ot.opt"), iniFileExact;
    bool buildFullPathSuccess =
        OTPaths::RelativeToCanonical(iniFileExact, configPath, optionsFile);
    OT_ASSERT_MSG(buildFullPathSuccess, "Unable to set Full Path");

    opt.processFile(iniFileExact.Get());
}

class DummyPassphraseCallback : public OTCallback
{
private:
    const std::string dummy_;

public:
    DummyPassphraseCallback(const std::string dummy)
        : dummy_(dummy)
    {
    }
    void runOne(const char*, OTPassword& password) const
    {
        password.setPassword(dummy_.c_str(), dummy_.size());
    }

    void runTwo(const char*, OTPassword& password) const
    {
        password.setPassword(dummy_.c_str(), dummy_.size());
    }
};

int Opentxs::processCommand(AnyOption& opt)
{
    string command = opt.getArgv(0);

    if (opt.getFlag("dummy-passphrase")) {
        // For automatic testing, set the password callback to
        // always return "test" as the password, not prompting the user.
        OTCaller* caller = new OTCaller;
        DummyPassphraseCallback* callback = new DummyPassphraseCallback("test");
        caller->setCallback(callback);
        OTAsymmetricKey::SetPasswordCaller(*caller);
    }

    if ("version" == command) {
        otOut << "opentxs " << OPENTXS_VERSION_STRING << "\n";
        otOut << "Copyright (C) 2014 Open Transactions Developers\n";
        return 0;
    }

    if ("list" == command) {
        otOut << "\nCommands:\n\n";
        for (int32_t i = 0; cmds[i] != nullptr; i++) {
            CmdBase& cmd = *cmds[i];
            otOut << (cmd.getCommand() + spaces18).substr(0, 18);
            if (i % 4 == 3) {
                otOut << "\n";
            }
        }
        otOut << "\n";
        return 0;
    }

    if ("help" == command) {
        // create category groups
        string categoryGroup[catLast];
        for (int i = 1; i < catLast; i++) {
            categoryGroup[i] = string("\n ") + categoryName[i] + ":\n";
        }

        // add commands to their category group
        otOut << "\nCommands:\n";
        for (int32_t i = 0; cmds[i] != nullptr; i++) {
            CmdBase& cmd = *cmds[i];
            categoryGroup[cmd.getCategory()] +=
                (cmd.getCommand() + spaces18).substr(0, 18) + cmd.getHelp() +
                "\n";
        }

        // print all category groups
        for (int i = 1; i < catLast; i++) {
            otOut << categoryGroup[i];
        }

        return 0;
    }

    for (int32_t i = 0; cmds[i] != nullptr; i++) {
        CmdBase& cmd = *cmds[i];
        if (command == cmd.getCommand()) {
            return runCommand(cmd);
        }
    }

    otOut << "Expecting a single opentxs command.\n";

    return -1;
}

int Opentxs::run(int argc, char* argv[])
{
    if (OTAPI_Wrap::OTAPI() == nullptr) return -1;

    OTAPI_Wrap::OTAPI()->LoadWallet();

    map<string, string> macros;
    vector<int> errorLineNumbers;
    vector<string> errorCommands;

    newArgc = argc;
    newArgv = argv;

    AnyOption opt;
    loadOptions(opt);
    opt.processCommandArgs(argc, argv);

    // is there a command on the command line?
    if (opt.getArgc() != 0) {
        expectFailure = false;
        return processCommand(opt);
    }

    int lineNumber = 0;
    bool echoCommand = opt.getFlag("echocommand") || opt.getFlag("test");
    bool echoExpand = opt.getFlag("echoexpand") || opt.getFlag("test");
    bool noPrompt = opt.getFlag("noprompt") || opt.getFlag("test");
    int processed = 0;
    while (true) {
        // get next command line from input stream
        if (!noPrompt) {
            cout << "\nopentxs> ";
        }
        string cmd;
        getline(cin, cmd);

        // end of file stops processing commands
        if (cin.eof()) {
            break;
        }

        lineNumber++;

        cmd = trim(cmd);
        if (echoCommand) {
            cout << cmd << endl;
        }

        // empty lines and lines starting with a hash character are seen as
        // comments
        if (cmd.size() == 0 || cmd[0] == '#') {
            continue;
        }

        // exit/quit the command loop?
        if ("exit" == cmd || "quit" == cmd) {
            break;
        }

        string originalCmd = cmd;

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
                   (isalnum(cmd[nameLength]) || cmd[nameLength] == '_')) {
                nameLength++;
            }
            string macroName = cmd.substr(0, nameLength);

            // skip whitespace
            size_t i = nameLength;
            while (i < cmd.length() && isspace(cmd[i])) {
                i++;
            }

            if (i == cmd.length() || cmd[i] != '=') {
                otOut << "\n\n***ERROR***\n"
                         "Expected macro definition of the form: "
                         "$macroName = macroValue\n"
                         "Command was: " << cmd;
                continue;
            }

            // remainder of line after trimming whitespace is macro value
            string macroValue = cmd.substr(i + 1);
            macros[macroName] = trim(macroValue);
            continue;
        }

        // now replace any macro in the command line with its value
        // unknown macro names will cause an error message instead of command
        // execution
        // note that all macro names are 'maximum munch'
        int expansions = 0;
        for (size_t macro = cmd.find_first_of("$"); macro != string::npos;
             macro = cmd.find_first_of("$", macro + 1)) {
            // first see if this is an escaped literal
            if (macro > 0 && cmd[macro - 1] == '\\') {
                continue;
            }

            // gather rest of macro name 'maximum munch'
            size_t macroEnd = macro + 1;
            while (macroEnd < cmd.length() &&
                   (isalnum(cmd[macroEnd]) || cmd[macroEnd] == '_')) {
                macroEnd++;
            }

            // has this macro been defined?
            string macroName = cmd.substr(macro, macroEnd - macro);
            auto found = macros.find(macroName);
            if (found == macros.end()) {
                otOut << "\n\n***ERROR***\n"
                         "Macro expansion failed.\n"
                         "Unknown macro: " << macroName
                      << "\n"
                         "Command was: " << cmd;
                expansions = 100;
                break;
            }

            string& macroValue = found->second;

            // limit to 100 expansions to avoid endless recusion loop
            expansions++;
            if (expansions > 100) {
                otOut << "\n\n***ERROR***\n"
                         "Macro expansion failed.\n"
                         "Too many expansions at macro: " << macroName
                      << "\n"
                         "Command was: " << cmd;
                break;
            }

            // limit to 10000 characters to avoid crazy recursive expansions
            if (cmd.length() + macroValue.length() > 10000) {
                otOut << "\n\n***ERROR***\n"
                         "Macro expansion failed.\n"
                         "Command length exceeded at macro: " << macroName
                      << "\n"
                         "Macro value is: " << macroValue
                      << "\n"
                         "Command was: " << cmd;
                expansions = 100;
                break;
            }

            // expand the command line
            cmd = cmd.substr(0, macro) + macroValue + cmd.substr(macroEnd);
        }

        if (echoExpand && cmd != originalCmd) {
            otOut << cmd << endl;
        }

        // skip command when anything during macro expansion failed
        if (expansions > 99) {
            continue;
        }

        // '!' indicates that we expect this command to fail
        //     which is very useful for running a test script
        expectFailure = cmd[0] == '!';

        // Parse command string into its separate parts so it can be passed as
        // an argc/argv combo
        // Whitespace separates args as usual.
        // To include whitespace in an arg surround the entire arg with double
        // quotes
        // An unterminated double-quoted arg will auto-terminate at end of line
        // All characters are taken literal except for: double quote, dollar
        // sign, and backslash
        // To take any character literal, precede it with a backslash
        vector<string> arguments;

        // add original command name
        arguments.push_back(argv[0]);

        // set up buffer that will receive the separate arguments
        char* buf = new char[cmd.length() + 1];
        char* arg = buf;

        // start at actual command
        size_t i = expectFailure ? 1 : 0;
        while (i < cmd.length()) {
            // skip any whitespace
            while (i < cmd.length() && isspace(cmd[i])) {
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
                while (i < cmd.length() && !isspace(cmd[i])) {
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
        newArgc = arguments.size();
        newArgv = new char* [newArgc];
        for (int i = 0; i < newArgc; i++) {
            newArgv[i] = const_cast<char*>(arguments[i].c_str());
        }

        // preprocess the command line
        AnyOption opt;
        loadOptions(opt);
        opt.processCommandArgs(newArgc, newArgv);

        cout << "\n";
        if (expectFailure != (0 != processCommand(opt))) {
            errorLineNumbers.push_back(lineNumber);
            errorCommands.push_back(originalCmd);
            otOut << "\n\n***ERROR***\n"
                  << (expectFailure ? "Expected command to fail.\nSucceeding"
                                    : "Failed") << " command was: " << cmd;
        }

        delete[] newArgv;
        delete[] buf;

        newArgc = 0;
        newArgv = nullptr;

        otOut << "\n\n";
        processed++;
    }

    int failed = errorLineNumbers.size();
    cout << "\n\n" << processed << " commands were processed.\n" << failed
         << " commands failed.\n" << endl;

    if (opt.getFlag("errorList") || opt.getFlag("test")) {
        for (size_t i = 0; i < errorLineNumbers.size(); i++) {
            cout << "\nFailed line " << errorLineNumbers[i] << ": "
                 << errorCommands[i] << endl;
        }
    }

    return failed == 0 ? 0 : -1;
}

int Opentxs::runCommand(CmdBase& cmd)
{
    auto argNames = cmd.extractArgumentNames();

    AnyOption options;
    for (size_t i = 0; i < argNames.size(); i++) {
        if (!options.findOption(argNames[i].c_str())) {
            options.setCommandOption(argNames[i].c_str());
        }
    }

    loadOptions(options);
    options.processCommandArgs(newArgc, newArgv);

    map<string, string> arguments;
    for (size_t i = 0; i < argNames.size(); i++) {
        const char* optionName = argNames[i].c_str();
        const char* value = options.getValue(optionName);
        if (value != nullptr) {
            arguments[argNames[i]] = value;
            continue;
        }

        static const string defaultPrefix = "default";
        static const string haveDefault[] = {"hisacct", "hisnym", "hispurse",
                                             "myacct",  "mynym",  "mypurse",
                                             "server",  ""};

        // can we get the argument value from the option file?
        for (int i = 0; haveDefault[i] != ""; i++) {
            if (haveDefault[i] == optionName) {
                value = options.getValue((defaultPrefix + optionName).c_str());
                if (value != nullptr) {
                    arguments[argNames[i]] = value;
                }
                break;
            }
        }
    }

    bool success = cmd.run(arguments);
    if (!success && !expectFailure) {
        cout << cmd.getUsage();
    }
    return success ? 0 : -1;
}
