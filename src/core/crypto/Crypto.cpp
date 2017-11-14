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

#include "opentxs/stdafx.hpp"

#include "opentxs/core/crypto/Crypto.hpp"

#include "opentxs/api/Native.hpp"
#include "opentxs/api/OT.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/Log.hpp"

#include <stdint.h>
#include <ostream>
#include <string>

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace opentxs
{

// TODO optimzation maybe this should be 10000 instead of 65535
#define OT_DEFAULT_ITERATION_COUNT 65535       // in bytes
#define OT_DEFAULT_SYMMETRIC_SALT_SIZE 8       // in bytes
#define OT_DEFAULT_SYMMETRIC_KEY_SIZE 32       // in bytes
#define OT_DEFAULT_SYMMETRIC_KEY_SIZE_MAX 64   // in bytes == 512 bits
#define OT_DEFAULT_SYMMETRIC_IV_SIZE 32        // in bytes
#define OT_DEFAULT_SYMMETRIC_BUFFER_SIZE 4096  // in bytes
#define OT_DEFAULT_PUBLIC_KEYSIZE 128          // in bytes == 4096 bits
#define OT_DEFAULT_PUBLIC_KEYSIZE_MAX 512      // in bytes == 1024 bits

#define OT_KEY_ITERATION_COUNT "iteration_count"
#define OT_KEY_SYMMETRIC_SALT_SIZE "symmetric_salt_size"
#define OT_KEY_SYMMETRIC_KEY_SIZE "symmetric_key_size"
#define OT_KEY_SYMMETRIC_KEY_SIZE_MAX "symmetric_key_size_max"
#define OT_KEY_SYMMETRIC_IV_SIZE "symmetric_iv_size"
#define OT_KEY_SYMMETRIC_BUFFER_SIZE "symmetric_buffer_size"
#define OT_KEY_PUBLIC_KEYSIZE "public_keysize"
#define OT_KEY_PUBLIC_KEYSIZE_MAX "public_keysize_max"

const int32_t* CryptoConfig::sp_nIterationCount = nullptr;
const int32_t* CryptoConfig::sp_nSymmetricSaltSize = nullptr;
const int32_t* CryptoConfig::sp_nSymmetricKeySize = nullptr;
const int32_t* CryptoConfig::sp_nSymmetricKeySizeMax = nullptr;
const int32_t* CryptoConfig::sp_nSymmetricIvSize = nullptr;
const int32_t* CryptoConfig::sp_nSymmetricBufferSize = nullptr;
const int32_t* CryptoConfig::sp_nPublicKeysize = nullptr;
const int32_t* CryptoConfig::sp_nPublicKeysizeMax = nullptr;

bool CryptoConfig::GetSetAll()
{
    if (!GetSetValue(
            OT_KEY_ITERATION_COUNT,
            OT_DEFAULT_ITERATION_COUNT,
            sp_nIterationCount))
        return false;
    if (!GetSetValue(
            OT_KEY_SYMMETRIC_SALT_SIZE,
            OT_DEFAULT_SYMMETRIC_SALT_SIZE,
            sp_nSymmetricSaltSize))
        return false;
    if (!GetSetValue(
            OT_KEY_SYMMETRIC_KEY_SIZE,
            OT_DEFAULT_SYMMETRIC_KEY_SIZE,
            sp_nSymmetricKeySize))
        return false;
    if (!GetSetValue(
            OT_KEY_SYMMETRIC_KEY_SIZE_MAX,
            OT_DEFAULT_SYMMETRIC_KEY_SIZE_MAX,
            sp_nSymmetricKeySizeMax))
        return false;
    if (!GetSetValue(
            OT_KEY_SYMMETRIC_IV_SIZE,
            OT_DEFAULT_SYMMETRIC_IV_SIZE,
            sp_nSymmetricIvSize))
        return false;
    if (!GetSetValue(
            OT_KEY_SYMMETRIC_BUFFER_SIZE,
            OT_DEFAULT_SYMMETRIC_BUFFER_SIZE,
            sp_nSymmetricBufferSize))
        return false;
    if (!GetSetValue(
            OT_KEY_PUBLIC_KEYSIZE,
            OT_DEFAULT_PUBLIC_KEYSIZE,
            sp_nPublicKeysize))
        return false;
    if (!GetSetValue(
            OT_KEY_PUBLIC_KEYSIZE_MAX,
            OT_DEFAULT_PUBLIC_KEYSIZE_MAX,
            sp_nPublicKeysizeMax))
        return false;

    return OT::App().Config().Save();
}

bool CryptoConfig::GetSetValue(
    std::string strKeyName,
    int32_t nDefaultValue,
    const int32_t*& out_nValue)

{
    if (strKeyName.empty()) return false;
    if (3 > strKeyName.size()) return false;

    {
        bool bIsNew = false;
        int64_t nValue = 0;
        OT::App().Config().CheckSet_long(
            "crypto", String(strKeyName), nDefaultValue, nValue, bIsNew);

        if (nullptr != out_nValue) {
            delete out_nValue;
            out_nValue = nullptr;
        }

        out_nValue =
            new int32_t(bIsNew ? nDefaultValue : static_cast<int32_t>(nValue));
    }

    return true;
}

const int32_t& CryptoConfig::GetValue(const int32_t*& pValue)
{
    if (nullptr == pValue) {
        if (!GetSetAll()) OT_FAIL;
    }
    if (nullptr == pValue) {
        OT_FAIL;
    }
    return *pValue;
}

uint32_t CryptoConfig::IterationCount() { return GetValue(sp_nIterationCount); }
uint32_t CryptoConfig::SymmetricSaltSize()
{
    return GetValue(sp_nSymmetricSaltSize);
}
uint32_t CryptoConfig::SymmetricKeySize()
{
    return GetValue(sp_nSymmetricKeySize);
}
uint32_t CryptoConfig::SymmetricKeySizeMax()
{
    return GetValue(sp_nSymmetricKeySizeMax);
}
uint32_t CryptoConfig::SymmetricIvSize()
{
    return GetValue(sp_nSymmetricIvSize);
}
uint32_t CryptoConfig::SymmetricBufferSize()
{
    return GetValue(sp_nSymmetricBufferSize);
}
uint32_t CryptoConfig::PublicKeysize() { return GetValue(sp_nPublicKeysize); }
uint32_t CryptoConfig::PublicKeysizeMax()
{
    return GetValue(sp_nPublicKeysizeMax);
}

void Crypto::Init() const { Init_Override(); }

void Crypto::Cleanup() const
{
    // Any crypto-related cleanup code NOT specific to OpenSSL (which is
    // handled in OpenSSL, a subclass) would go here.
    //
    Cleanup_Override();
}

// virtual (Should never get called.)
void Crypto::Init_Override() const
{
    otErr << "Crypto::Init_Override: ERROR: This function should NEVER be "
             "called (you should be overriding it...)\n";
}

// virtual (Should never get called.)
void Crypto::Cleanup_Override() const
{
    otErr << "Crypto::Cleanup_Override: ERROR: This function should NEVER be "
             "called (you should be overriding it...)\n";
}
}  // namespace opentxs
