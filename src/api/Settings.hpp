// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/Settings.cpp"

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
namespace api
{
class Legacy;
}  // namespace api

class Factory;
}  // namespace opentxs

namespace opentxs::api::implementation
{
class Settings final : public api::Settings
{
public:
    void SetConfigFilePath(const String& strConfigFilePath) const final;
    bool HasConfigFilePath() const final;

    // Core (Public Load and Save)
    bool Load() const final;
    bool Save() const final;

    const Flag& IsLoaded() const final;

    // Configuration Helpers
    //

    // Core (Reset Config, and Check if Config is empty)
    bool IsEmpty() const final;

    // Check Only (get value of key from configuration, if the key exists, then
    // out_bKeyExist will be true.)
    bool Check_str(
        const String& strSection,
        const String& strKey,
        String& out_strResult,
        bool& out_bKeyExist) const final;
    bool Check_long(
        const String& strSection,
        const String& strKey,
        std::int64_t& out_lResult,
        bool& out_bKeyExist) const final;
    bool Check_bool(
        const String& strSection,
        const String& strKey,
        bool& out_bResult,
        bool& out_bKeyExist) const final;

    // Set Only (set new or update value, out_bNewOrUpdate will be true if the
    // value changes.)
    bool Set_str(
        const String& strSection,
        const String& strKey,
        const String& strValue,
        bool& out_bNewOrUpdate,
        const String& strComment = String::Factory()) const final;
    bool Set_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lValue,
        bool& out_bNewOrUpdate,
        const String& strComment = String::Factory()) const final;
    bool Set_bool(
        const String& strSection,
        const String& strKey,
        const bool& bValue,
        bool& out_bNewOrUpdate,
        const String& strComment = String::Factory()) const final;

    // Check for a Section, if the section dosn't exist, it will be made and
    // out_bIsNewSection will be true.)
    bool CheckSetSection(
        const String& strSection,
        const String& strComment,
        bool& out_bIsNewSection) const final;

    // Check for Key, and returns if the key exists, otherwise will set the
    // default key. If the default key is set, then out_bIsNew will be true.)
    bool CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        std::string& out_strResult,
        bool& out_bIsNew,
        const String& strComment = String::Factory()) const final;
    bool CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        String& out_strResult,
        bool& out_bIsNew,
        const String& strComment = String::Factory()) const final;
    bool CheckSet_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lDefault,
        std::int64_t& out_lResult,
        bool& out_bIsNew,
        const String& strComment = String::Factory()) const final;
    bool CheckSet_bool(
        const String& strSection,
        const String& strKey,
        const bool& bDefault,
        bool& out_bResult,
        bool& out_bIsNew,
        const String& strComment = String::Factory()) const final;

    // Set Option helper function for setting bool's
    bool SetOption_bool(
        const String& strSection,
        const String& strKey,
        bool& bVariableName) const final;

    bool Reset() final;

    ~Settings() final;

private:
    friend opentxs::Factory;

    class SettingsPvt;

    const api::Legacy& legacy_;
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

    explicit Settings(
        const api::Legacy& legacy,
        const String& strConfigFilePath);
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
};
}  // namespace opentxs::api::implementation
