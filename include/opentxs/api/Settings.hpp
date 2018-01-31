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

#ifndef OPENTXS_API_SETTINGS_HPP
#define OPENTXS_API_SETTINGS_HPP

#include "opentxs/Version.hpp"

#include "opentxs/core/String.hpp"

#include <atomic>
#include <stdint.h>
#include <memory>
#include <string>

namespace opentxs
{

class Log;
class OTDataFolder;
class OTFolders;
class OTPaths;

namespace api
{

namespace implementation
{
class Native;
}

class Settings
{
private:
    friend class implementation::Native;
    friend class opentxs::Log;
    friend class opentxs::OTDataFolder;
    friend class opentxs::OTFolders;
    friend class opentxs::OTPaths;

    class SettingsPvt;

    std::unique_ptr<SettingsPvt> pvt_;
    mutable std::atomic<bool> b_Loaded{false};

    mutable String m_strConfigurationFileExactPath;

    // Core (Load and Save)
    EXPORT bool Load(const String& strConfigurationFileExactPath) const;
    EXPORT bool Save(const String& strConfigurationFileExactPath) const;

    // Log (log to Output in a well-formated way).
    EXPORT bool LogChange_str(
        const String& strSection,
        const String& strKey,
        const String& strValue) const;

    EXPORT bool Init() const;
    EXPORT bool Reset() const;

    EXPORT Settings();
    EXPORT explicit Settings(const String& strConfigFilePath);
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

public:
    EXPORT void SetConfigFilePath(const String& strConfigFilePath) const;
    EXPORT bool HasConfigFilePath() const;

    // Core (Public Load and Save)
    EXPORT bool Load() const;
    EXPORT bool Save() const;

    EXPORT const std::atomic<bool>& IsLoaded() const;

    // Configuration Helpers
    //

    // Core (Reset Config, and Check if Config is empty)
    EXPORT bool IsEmpty() const;

    // Check Only (get value of key from configuration, if the key exists, then
    // out_bKeyExist will be true.)
    EXPORT bool Check_str(
        const String& strSection,
        const String& strKey,
        String& out_strResult,
        bool& out_bKeyExist) const;
    EXPORT bool Check_long(
        const String& strSection,
        const String& strKey,
        int64_t& out_lResult,
        bool& out_bKeyExist) const;
    EXPORT bool Check_bool(
        const String& strSection,
        const String& strKey,
        bool& out_bResult,
        bool& out_bKeyExist) const;

    // Set Only (set new or update value, out_bNewOrUpdate will be true if the
    // value changes.)
    EXPORT bool Set_str(
        const String& strSection,
        const String& strKey,
        const String& strValue,
        bool& out_bNewOrUpdate,
        const String& strComment = "") const;
    EXPORT bool Set_long(
        const String& strSection,
        const String& strKey,
        const int64_t& lValue,
        bool& out_bNewOrUpdate,
        const String& strComment = "") const;
    EXPORT bool Set_bool(
        const String& strSection,
        const String& strKey,
        const bool& bValue,
        bool& out_bNewOrUpdate,
        const String& strComment = "") const;

    // Check for a Section, if the section dosn't exist, it will be made and
    // out_bIsNewSection will be true.)
    EXPORT bool CheckSetSection(
        const String& strSection,
        const String& strComment,
        bool& out_bIsNewSection) const;

    // Check for Key, and returns if the key exists, otherwise will set the
    // default key. If the default key is set, then out_bIsNew will be true.)
    EXPORT bool CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        std::string& out_strResult,
        bool& out_bIsNew,
        const String& strComment = "") const;
    EXPORT bool CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        String& out_strResult,
        bool& out_bIsNew,
        const String& strComment = "") const;
    EXPORT bool CheckSet_long(
        const String& strSection,
        const String& strKey,
        const int64_t& lDefault,
        int64_t& out_lResult,
        bool& out_bIsNew,
        const String& strComment = "") const;
    EXPORT bool CheckSet_bool(
        const String& strSection,
        const String& strKey,
        const bool& bDefault,
        bool& out_bResult,
        bool& out_bIsNew,
        const String& strComment = "") const;

    // Set Option helper function for setting bool's
    EXPORT bool SetOption_bool(
        const String& strSection,
        const String& strKey,
        bool& bVariableName) const;

    EXPORT ~Settings();
};
}  // namespace api
}  // namespace opentxs

#endif  // OPENTXS_API_SETTINGS_HPP
