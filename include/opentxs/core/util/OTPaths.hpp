/************************************************************
 *
 * OTPaths.hpp
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
        OTString& out_InstallFolderPath);

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
