// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifdef _WIN32

#include "stdafx.hpp"

#include "opentxs/core/util/OTWindowsRegistryTools.hpp"

LONG WindowsRegistryTools::GetDWORDRegKey(
    HKEY hKey,
    const std::wstring& strValueName,
    DWORD& nValue,
    DWORD nDefaultValue)
{
    nValue = nDefaultValue;
    DWORD dwBufferSize(sizeof(DWORD));
    DWORD nResult(0);
    LONG nError = ::RegQueryValueExW(
        hKey,
        strValueName.c_str(),
        0,
        nullptr,
        reinterpret_cast<LPBYTE>(&nResult),
        &dwBufferSize);
    if (ERROR_SUCCESS == nError) { nValue = nResult; }
    return nError;
}

LONG WindowsRegistryTools::GetBoolRegKey(
    HKEY hKey,
    const std::wstring& strValueName,
    bool& bValue,
    bool bDefaultValue)
{
    DWORD nDefValue((bDefaultValue) ? 1 : 0);
    DWORD nResult(nDefValue);
    LONG nError = GetDWORDRegKey(hKey, strValueName, nResult, nDefValue);
    if (ERROR_SUCCESS == nError) { bValue = (nResult != 0) ? true : false; }
    return nError;
}

LONG WindowsRegistryTools::GetStringRegKey(
    HKEY hKey,
    const std::wstring& strValueName,
    std::wstring& strValue,
    const std::wstring& strDefaultValue)
{
    strValue = strDefaultValue;
    WCHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
    nError = RegQueryValueExW(
        hKey,
        strValueName.c_str(),
        0,
        nullptr,
        (LPBYTE)szBuffer,
        &dwBufferSize);
    if (ERROR_SUCCESS == nError) { strValue = szBuffer; }
    return nError;
}

#endif
