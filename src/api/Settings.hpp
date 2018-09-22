// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::implementation
{
class Settings : virtual public api::Settings
{
public:
    void SetConfigFilePath(const String& strConfigFilePath) const override;
    bool HasConfigFilePath() const override;

    // Core (Public Load and Save)
    bool Load() const override;
    bool Save() const override;

    const Flag& IsLoaded() const override;

    // Configuration Helpers
    //

    // Core (Reset Config, and Check if Config is empty)
    bool IsEmpty() const override;

    // Check Only (get value of key from configuration, if the key exists, then
    // out_bKeyExist will be true.)
    bool Check_str(
        const String& strSection,
        const String& strKey,
        String& out_strResult,
        bool& out_bKeyExist) const override;
    bool Check_long(
        const String& strSection,
        const String& strKey,
        std::int64_t& out_lResult,
        bool& out_bKeyExist) const override;
    bool Check_bool(
        const String& strSection,
        const String& strKey,
        bool& out_bResult,
        bool& out_bKeyExist) const override;

    // Set Only (set new or update value, out_bNewOrUpdate will be true if the
    // value changes.)
    bool Set_str(
        const String& strSection,
        const String& strKey,
        const String& strValue,
        bool& out_bNewOrUpdate,
        const String& strComment = String::Factory()) const override;
    bool Set_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lValue,
        bool& out_bNewOrUpdate,
        const String& strComment = String::Factory()) const override;
    bool Set_bool(
        const String& strSection,
        const String& strKey,
        const bool& bValue,
        bool& out_bNewOrUpdate,
        const String& strComment = String::Factory()) const override;

    // Check for a Section, if the section dosn't exist, it will be made and
    // out_bIsNewSection will be true.)
    bool CheckSetSection(
        const String& strSection,
        const String& strComment,
        bool& out_bIsNewSection) const override;

    // Check for Key, and returns if the key exists, otherwise will set the
    // default key. If the default key is set, then out_bIsNew will be true.)
    bool CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        std::string& out_strResult,
        bool& out_bIsNew,
        const String& strComment = String::Factory()) const override;
    bool CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        String& out_strResult,
        bool& out_bIsNew,
        const String& strComment = String::Factory()) const override;
    bool CheckSet_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lDefault,
        std::int64_t& out_lResult,
        bool& out_bIsNew,
        const String& strComment = String::Factory()) const override;
    bool CheckSet_bool(
        const String& strSection,
        const String& strKey,
        const bool& bDefault,
        bool& out_bResult,
        bool& out_bIsNew,
        const String& strComment = String::Factory()) const override;

    // Set Option helper function for setting bool's
    bool SetOption_bool(
        const String& strSection,
        const String& strKey,
        bool& bVariableName) const override;

    bool Reset() override;

    ~Settings() override;

private:
    friend opentxs::Factory;

    class SettingsPvt;

    std::unique_ptr<SettingsPvt> pvt_;
    mutable OTFlag loaded_;
    mutable std::recursive_mutex lock_;

    mutable OTString m_strConfigurationFileExactPath;

    // Core (Load and Save)
    bool Load(const String& strConfigurationFileExactPath) const;
    bool Save(const String& strConfigurationFileExactPath) const;

    // Log (log to Output in a well-formated way).
    bool LogChange_str(
        const String& strSection,
        const String& strKey,
        const String& strValue) const;

    bool Init();

    Settings();
    explicit Settings(const String& strConfigFilePath);
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
};
}  // namespace opentxs::api::implementation
