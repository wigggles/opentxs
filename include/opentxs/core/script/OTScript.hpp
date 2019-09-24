// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_SCRIPT_OTSCRIPT_HPP
#define OPENTXS_CORE_SCRIPT_OTSCRIPT_HPP

#include "opentxs/Forward.hpp"

#include <map>
#include <string>
#include <memory>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)  // warning C4702: unreachable code
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace opentxs
{

class OTParty;
class OTPartyAccount;
class String;
class OTVariable;

typedef std::map<std::string, OTParty*> mapOfParties;
typedef std::map<std::string, OTPartyAccount*> mapOfPartyAccounts;
typedef std::map<std::string, OTVariable*> mapOfVariables;

// A script should be "Dumb", meaning that you just stick it with its
// parties and other resources, and it EXPECTS them to be the correct
// ones.  It uses them low-level style.
//
// Any verification should be done at a higher level, in OTSmartContract.
// There, multiple parties might be loaded, as well as multiple scripts
// (clauses) and that is where the proper resources, accounts, etc are
// instantiated and validated before any use.
//
// Thus by the time you get down to OTScript, all that validation is already
// done.  The programmatic user will interact with OTSmartContract, likely,
// and not with OTScript itself.
//
class OTScript
{
protected:
    std::string m_str_script;            // the script itself.
    std::string m_str_display_filename;  // for error handling, there is option
                                         // to set this string for display.
    mapOfParties m_mapParties;  // no need to clean this up. Script doesn't own
                                // the parties, just references them.
    mapOfPartyAccounts m_mapAccounts;  // no need to clean this up. Script
                                       // doesn't own the accounts, just
                                       // references them.
    mapOfVariables m_mapVariables;  // no need to clean this up. Script doesn't
                                    // own the variables, just references them.

    // List
    // Construction -- Destruction
public:
    OTScript();
    OTScript(const String& strValue);
    OTScript(const char* new_string);
    OTScript(const char* new_string, size_t sizeLength);
    OTScript(const std::string& new_string);

    virtual ~OTScript();

    EXPORT void SetScript(const String& strValue);
    EXPORT void SetScript(const char* new_string);
    EXPORT void SetScript(const char* new_string, size_t sizeLength);
    EXPORT void SetScript(const std::string& new_string);

    void SetDisplayFilename(std::string str_display_filename)
    {
        m_str_display_filename = str_display_filename;
    }

    // The same OTSmartContract that loads all the clauses (scripts) will
    // also load all the parties, so it will call this function whenever before
    // it
    // needs to actually run a script.
    //
    // NOTE: OTScript does NOT take ownership of the party, since there could be
    // multiple scripts (with all scripts and parties being owned by a
    // OTSmartContract.)
    // Therefore it's ASSUMED that the owner OTSmartContract will handle all the
    // work of
    // cleaning up the mess!  theParty is passed as reference to insure it
    // already exists.
    //
    void AddParty(std::string str_party_name, OTParty& theParty);
    void AddAccount(std::string str_acct_name, OTPartyAccount& theAcct);
    EXPORT void AddVariable(std::string str_var_name, OTVariable& theVar);
    EXPORT OTVariable* FindVariable(std::string str_var_name);
    EXPORT void RemoveVariable(OTVariable& theVar);

    // Note: any relevant assets or asset accounts are listed by their owner /
    // contributor
    // parties. Therefore there's no need to separately input any accounts or
    // assets to
    // a script, since the necessary ones are already present inside their
    // respective parties.

    virtual bool ExecuteScript(OTVariable* pReturnVar = nullptr);
};

EXPORT std::shared_ptr<OTScript> OTScriptFactory(
    const std::string& script_type = "");
EXPORT std::shared_ptr<OTScript> OTScriptFactory(
    const std::string& script_type,
    const std::string& script_contents);

}  // namespace opentxs

#endif
