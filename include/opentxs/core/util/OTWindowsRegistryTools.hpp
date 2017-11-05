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

#ifndef OPENTXS_CORE_UTIL_WINDOWSREGISTRYTOOLS_HPP
#define OPENTXS_CORE_UTIL_WINDOWSREGISTRYTOOLS_HPP

#include "opentxs/Version.hpp"

#ifdef _WIN32

#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include <xstring>

class WindowsRegistryTools
{
public:
    LONG GetDWORDRegKey(
        HKEY hKey,
        const std::wstring& strValueName,
        DWORD& nValue,
        DWORD nDefaultValue);
    LONG GetBoolRegKey(
        HKEY hKey,
        const std::wstring& strValueName,
        bool& bValue,
        bool bDefaultValue);
    LONG GetStringRegKey(
        HKEY hKey,
        const std::wstring& strValueName,
        std::wstring& strValue,
        const std::wstring& strDefaultValue);
};

#endif

#endif  // OPENTXS_CORE_UTIL_WINDOWSREGISTRYTOOLS_HPP
