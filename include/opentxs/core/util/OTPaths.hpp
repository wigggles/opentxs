// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_UTIL_OTPATHS_HPP
#define OPENTXS_CORE_UTIL_OTPATHS_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/api/Settings.hpp"
#include "opentxs/core/String.hpp"

#include <cstdint>

// All directories have a trailing "/" while files do not. <== remember to
// enforce this!!!
namespace opentxs
{
class OTPaths
{
public:
    EXPORT static const String& AppDataFolder();
    EXPORT static bool AppendFile(
        String& out_strPath,
        const String& strBasePath,
        const String& strFileName);  // the trailing "/" is optional for the
                                     // strFolderName
    EXPORT static bool AppendFolder(
        String& out_strPath,
        const String& strBasePath,
        const String& strFolderName);  // the trailing "/" is optional for the
                                       // strFolderName
    EXPORT static bool BuildFolderPath(
        const String& strFolderPath,
        bool& out_bFolderCreated);  // will build all the folders to a path.
                                    // Will return alse if unable to build path.
    EXPORT static bool BuildFilePath(
        const String& strFolderPath,
        bool& out_bFolderCreated);  // will build all the folders up to the
                                    // file. Will return false if unable to
                                    // build path.
    EXPORT static bool ConfirmCreateFolder(
        const String& strExactPath,
        bool& out_Exists,
        bool& out_IsNew);
    EXPORT static bool FileExists(
        const String& strFilePath,
        std::int64_t& nFileLength);
    EXPORT static bool PathExists(const String& strPath);

    EXPORT ~OTPaths();

private:
    static api::Settings* s_settings;

    static OTString s_strAppBinaryFolder;
    static OTString s_strHomeFolder;
    static OTString s_strAppDataFolder;
    static OTString s_strGlobalConfigFile;
    static OTString s_strPrefixFolder;
    static OTString s_strScriptsFolder;

    static const String& AppBinaryFolder();
    static void ConfigureDefaultSettings();
    static bool FixPath(
        const String& strPath,
        String& out_strFixedPath,
        const bool& bIsFolder);
    static bool FolderExists(const String& strFolderPath);
    static bool GetHomeFromSystem(String& out_strHomeFolder);
    static const String& GlobalConfigFile();
    static bool LoadSetPrefixFolder(
        api::Settings& config = *s_settings,
        const String& strPrefixFolder = String::Factory());
    static bool LoadSetScriptsFolder(
        api::Settings& config = *s_settings,
        const String& strScriptsFolder = String::Factory(),
        const bool& bIsRelative = true);
    static const String& HomeFolder();
    static const String& PrefixFolder();
    static const String& ScriptsFolder();
    static void SetAppBinaryFolder(const String& strLocation);
    static void SetHomeFolder(const String& strLocation);
    static bool ToReal(
        const String& strExactPath,
        String& out_strCanonicalPath);
#ifdef _WIN32
    static bool Win_GetInstallFolderFromRegistry(String& out_InstallFolderPath);
#endif

    OTPaths();
};  // class OTPaths
}  // namespace opentxs
#endif
