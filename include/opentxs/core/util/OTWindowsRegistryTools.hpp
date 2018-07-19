// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_UTIL_WINDOWSREGISTRYTOOLS_HPP
#define OPENTXS_CORE_UTIL_WINDOWSREGISTRYTOOLS_HPP

#include "opentxs/Forward.hpp"

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
