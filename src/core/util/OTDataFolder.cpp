/************************************************************
 *
 *  OTDataFolder.cpp
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

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/util/OTDataFolder.hpp>
#include <opentxs/core/util/OTPaths.hpp>
#include <opentxs/core/OTLog.hpp>

#ifdef _WIN32
#include <direct.h>
#include <shlobj.h>
#endif

#ifndef _WIN32
#include <libgen.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#ifdef TARGET_OS_MAC
#include <mach-o/dyld.h>
#include <limits.h>
#endif

#define CONFIG_FILE_EXT ".cfg"
#define DATA_FOLDER_EXT "_data"
#define OT_CONFIG_ISRELATIVE "_is_relative"

namespace opentxs
{

OTDataFolder* OTDataFolder::pDataFolder;

bool OTDataFolder::Init(const String& strThreadContext)
{
    if (nullptr != pDataFolder)
        return true; // we already have a data dir setup.

    if (!strThreadContext.Exists()) {
        otErr << __FUNCTION__ << ": Null: "
              << "strThreadContext"
              << " passed in!\n";
        OT_FAIL;
    }
    if (3 > strThreadContext.GetLength()) {
        otErr << __FUNCTION__ << ": Too Short: "
              << "strThreadContext"
              << " !\n";
        OT_FAIL;
    }

    pDataFolder = new OTDataFolder; // make the new instance

    pDataFolder->m_bInitialized = false;

    // setup the config instance.
    OTSettings* pSettings(new OTSettings(OTPaths::GlobalConfigFile()));
    pSettings->Reset();
    if (!pSettings->Load()) return false;

    // setup the RelativeKey
    String l_strRelativeKey("");
    l_strRelativeKey.Format("%s%s", strThreadContext.Get(),
                            OT_CONFIG_ISRELATIVE);

    bool l_IsRelative(false), l_Exist(false);
    String l_strFolderName(""), l_strDataConifgFilename("");

    // check the config for an existing configuration.
    if (!pSettings->Check_bool("data_path", l_strRelativeKey, l_IsRelative,
                               l_Exist)) {
        return false;
    } // is data folder relative

    if (l_Exist) {
        if (!pSettings->Check_str("data_path", strThreadContext,
                                  l_strFolderName, l_Exist)) {
            return false;
        } // what is the data folder

        if (l_Exist) {
            if (!pSettings->Check_str("data_config", strThreadContext,
                                      l_strDataConifgFilename, l_Exist)) {
                return false;
            } // what is config file name

            if (l_Exist) {
                if (l_IsRelative) // data folder path
                {
                    if (!OTPaths::AppendFolder(pDataFolder->m_strDataFolderPath,
                                               OTPaths::AppDataFolder(),
                                               l_strFolderName)) {
                        return false;
                    }
                }
                else {
                    pDataFolder->m_strDataFolderPath = l_strFolderName;
                }

                // data config file path.
                if (!OTPaths::AppendFile(pDataFolder->m_strDataConifgFilePath,
                                         OTPaths::AppDataFolder(),
                                         l_strDataConifgFilename)) {
                    return false;
                }

                pDataFolder->m_bInitialized = true;
                return true;
            }
        }
    }

    // if we get here we do not have a valid config, lets set one.

    // setup the default conifg file-name;
    l_strFolderName.Format("%s%s", strThreadContext.Get(), DATA_FOLDER_EXT);
    l_strDataConifgFilename.Format("%s%s", strThreadContext.Get(),
                                   CONFIG_FILE_EXT);

    if (!pSettings->Set_bool("data_path", l_strRelativeKey, true, l_Exist)) {
        return false;
    }
    if (!pSettings->Set_str("data_path", strThreadContext, l_strFolderName,
                            l_Exist)) {
        return false;
    }
    if (!pSettings->Set_str("data_config", strThreadContext,
                            l_strDataConifgFilename, l_Exist)) {
        return false;
    }

    if (!OTPaths::AppendFolder(pDataFolder->m_strDataFolderPath,
                               OTPaths::AppDataFolder(), l_strFolderName)) {
        return false;
    }
    if (!OTPaths::AppendFile(pDataFolder->m_strDataConifgFilePath,
                             OTPaths::AppDataFolder(),
                             l_strDataConifgFilename)) {
        return false;
    }

    // save config
    if (!pSettings->Save()) return false;
    pSettings->Reset();

    if (nullptr != pSettings) delete pSettings;
    pSettings = nullptr;

    // have set the default dir, now returning true;

    pDataFolder->m_bInitialized = true;
    return true;
}

bool OTDataFolder::IsInitialized()
{
    if (nullptr == pDataFolder)
        return false; // we already have a data dir setup.

    return pDataFolder->m_bInitialized;
}

bool OTDataFolder::Cleanup()
{
    if (nullptr != pDataFolder) {
        delete pDataFolder;
        pDataFolder = nullptr;
        return true;
    }
    else {
        pDataFolder = nullptr;
        return false;
    }
}

String OTDataFolder::Get()
{
    if (!OTDataFolder::IsInitialized()) {
        OT_FAIL;
    }

    String strDataFolder = "";
    if (OTDataFolder::Get(strDataFolder)) {
        return strDataFolder;
    }
    else {
        strDataFolder = "";
        return strDataFolder;
    }
}

bool OTDataFolder::Get(String& strDataFolder)
{
    if (nullptr != pDataFolder) {
        if (true == pDataFolder->m_bInitialized) {
            if (pDataFolder->m_strDataFolderPath.Exists()) {
                strDataFolder = pDataFolder->m_strDataFolderPath;
                return true;
            }
        }
    }

    return false;
}

bool OTDataFolder::GetConfigFilePath(String& strConfigFilePath)
{
    if (nullptr != pDataFolder) {
        if (true == pDataFolder->m_bInitialized) {
            if (pDataFolder->m_strDataConifgFilePath.Exists()) {
                strConfigFilePath = pDataFolder->m_strDataConifgFilePath;
                return true;
            }
        }
    }

    return false;
}

} // namespace opentxs
