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

#ifndef OPENTXS_CORE_OTPATHS_HPP
#define OPENTXS_CORE_OTPATHS_HPP

#include <opentxs/core/OTSettings.hpp>

// All directories have a trailing "/" while files do not. <== remember to
// enforce this!!!

namespace opentxs
{

class OTPaths
{
private:
    EXPORT OTPaths();

    static OTSettings s_settings;

    static String s_strAppBinaryFolder;
    static String s_strHomeFolder;
    static String s_strAppDataFolder;
    static String s_strGlobalConfigFile;
    static String s_strPrefixFolder;
    static String s_strScriptsFolder;

public:
    EXPORT ~OTPaths();

    EXPORT static const String& AppBinaryFolder(); // Adding this for Mac,
                                                   // since it's sandboxed.
                                                   // (Don't want to put
                                                   // scripts in data folder.)
    EXPORT static void SetAppBinaryFolder(String strLocation); // Note:
                                                               // Android
                                                               // should set
                                                               // this as the
                                                               // res/raw
                                                               // folder.

    EXPORT static const String& HomeFolder(); // Adding this for Android,
                                              // since it's sandboxed. Android
                                              // will provide its own data
                                              // folder here.
    EXPORT static void SetHomeFolder(String strLocation); // The AppDataFolder
                                                          // (below) will be
                                                          // created from this
                                                          // folder, plus .ot
                                                          // or whatever.

    EXPORT static const String& AppDataFolder();    // eg. /home/user/.ot/
                                                    // (auto).
    EXPORT static const String& GlobalConfigFile(); // ie. AppDataFolder() +
                                                    // ot_config.cfg
    EXPORT static const String& PrefixFolder();     // If not set, will run
    // LoadSetPrefixFolder with
    // default values.
    EXPORT static const String& ScriptsFolder(); // If not set, will run
                                                 // LoadSetScriptsFolder with
                                                 // default values.

    // The LoadSet Functions will update the static values.

    EXPORT static bool LoadSetPrefixFolder  // eg. /usr/local/  (cannot be
                                            // relative);
        (OTSettings& config = s_settings,   // optional
         const String& strPrefixFolder = "" // optional
         // const bool& bIsRelative = false
         );

    EXPORT static bool LoadSetScriptsFolder   // ie. PrefixFolder() + [if (NOT
                                              // Android) "lib/opentxs/" ]
        (OTSettings& config = s_settings,     // optional
         const String& strScriptsFolder = "", // optional
         const bool& bIsRelative = true       // optional
         );

    EXPORT static bool Get(OTSettings& config, const String& strSection,
                           const String& strKey, String& out_strVar,
                           bool& out_bIsRelative, bool& out_bKeyExist);

    EXPORT static bool Set(OTSettings& config, const String& strSection,
                           const String& strKey, const String& strValue,
                           const bool& bIsRelative, bool& out_bIsNewOrUpdated,
                           const String& strComment = "");

    EXPORT static bool FixPath(const String& strPath, String& out_strFixedPath,
                               const bool& bIsFolder);
    EXPORT static bool PathExists(const String& strPath); // returns true if
                                                          // path exists.

    EXPORT static bool FileExists(const String& strFilePath,
                                  int64_t& nFileLength); // returns true if file
                                                         // exists and its
                                                         // length.
    EXPORT static bool FolderExists(const String& strFolderPath); // returns
                                                                  // true if
                                                                  // folder
                                                                  // exists

    EXPORT static bool ConfirmCreateFolder(const String& strExactPath,
                                           bool& out_Exists, bool& out_IsNew);

    EXPORT static bool ToReal(const String& strExactPath,
                              String& out_strCanonicalPath);
    EXPORT static bool GetHomeFromSystem(String& out_strHomeFolder);

#ifdef _WIN32

    EXPORT static bool Win_GetInstallFolderFromRegistry(
        String& out_InstallFolderPath);

#endif

    // High Level Helper Functions
    EXPORT static bool AppendFolder(
        String& out_strPath, const String& strBasePath,
        const String& strFolderName); // the trailing "/" is optional for the
                                      // strFolderName
    EXPORT static bool AppendFile(String& out_strPath,
                                  const String& strBasePath,
                                  const String& strFileName); // the trailing
                                                              // "/" is
                                                              // optional for
                                                              // the
                                                              // strFolderName

    EXPORT static bool RelativeToCanonical(String& out_strCanonicalPath,
                                           const String& strBasePath,
                                           const String& strRelativePath);

    EXPORT static bool BuildFolderPath(const String& strFolderPath,
                                       bool& out_bFolderCreated); // will build
                                                                  // all the
                                                                  // folders to
                                                                  // a path.
                                                                  // Will return
                                                                  // false if
                                                                  // unable to
                                                                  // build path.
    EXPORT static bool BuildFilePath(const String& strFolderPath,
                                     bool& out_bFolderCreated); // will build
                                                                // all the
                                                                // folders up to
                                                                // the file.
                                                                // Will return
                                                                // false if
                                                                // unable to
                                                                // build path.

private:
    static void ConfigureDefaultSettings();
}; // class OTPaths

} // namespace opentxs

#endif // OPENTXS_CORE_OTPATHS_HPP
