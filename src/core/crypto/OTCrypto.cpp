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

#include <opentxs/core/crypto/OTCrypto.hpp>

#include <opentxs/core/stdafx.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/util/OTPaths.hpp>

extern "C" {
#ifdef _WIN32
#else
#include <sys/resource.h>
#endif
}

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace opentxs
{

// class OTCrypto
//
// To someday get us to the point where we can easily swap crypto libs.
// For now, just for static init / cleanup functions we can call from
// OTLog Init/Cleanup, and move the more "crypto" related stuff to this file.
//

// todo optimzation maybe this should be 10000 instead of 65535
//
#define OT_DEFAULT_ITERATION_COUNT 65535      // in bytes
#define OT_DEFAULT_SYMMETRIC_SALT_SIZE 8      // in bytes
#define OT_DEFAULT_SYMMETRIC_KEY_SIZE 16      // in bytes
#define OT_DEFAULT_SYMMETRIC_KEY_SIZE_MAX 64  // in bytes == 512 bits
#define OT_DEFAULT_SYMMETRIC_IV_SIZE 16       // in bytes
#define OT_DEFAULT_SYMMETRIC_BUFFER_SIZE 4096 // in bytes
#define OT_DEFAULT_PUBLIC_KEYSIZE 128         // in bytes == 4096 bits
#define OT_DEFAULT_PUBLIC_KEYSIZE_MAX 512     // in bytes == 1024 bits

#define OT_KEY_ITERATION_COUNT "iteration_count"
#define OT_KEY_SYMMETRIC_SALT_SIZE "symmetric_salt_size"
#define OT_KEY_SYMMETRIC_KEY_SIZE "symmetric_key_size"
#define OT_KEY_SYMMETRIC_KEY_SIZE_MAX "symmetric_key_size_max"
#define OT_KEY_SYMMETRIC_IV_SIZE "symmetric_iv_size"
#define OT_KEY_SYMMETRIC_BUFFER_SIZE "symmetric_buffer_size"
#define OT_KEY_PUBLIC_KEYSIZE "public_keysize"
#define OT_KEY_PUBLIC_KEYSIZE_MAX "public_keysize_max"

const int32_t* OTCryptoConfig::sp_nIterationCount = nullptr;
const int32_t* OTCryptoConfig::sp_nSymmetricSaltSize = nullptr;
const int32_t* OTCryptoConfig::sp_nSymmetricKeySize = nullptr;
const int32_t* OTCryptoConfig::sp_nSymmetricKeySizeMax = nullptr;
const int32_t* OTCryptoConfig::sp_nSymmetricIvSize = nullptr;
const int32_t* OTCryptoConfig::sp_nSymmetricBufferSize = nullptr;
const int32_t* OTCryptoConfig::sp_nPublicKeysize = nullptr;
const int32_t* OTCryptoConfig::sp_nPublicKeysizeMax = nullptr;

bool OTCryptoConfig::GetSetAll()
{
    OTSettings config(OTPaths::GlobalConfigFile());

    config.Reset();

    if (!config.Load()) return false;

    if (!GetSetValue(config, OT_KEY_ITERATION_COUNT, OT_DEFAULT_ITERATION_COUNT,
                     sp_nIterationCount))
        return false;
    if (!GetSetValue(config, OT_KEY_SYMMETRIC_SALT_SIZE,
                     OT_DEFAULT_SYMMETRIC_SALT_SIZE, sp_nSymmetricSaltSize))
        return false;
    if (!GetSetValue(config, OT_KEY_SYMMETRIC_KEY_SIZE,
                     OT_DEFAULT_SYMMETRIC_KEY_SIZE, sp_nSymmetricKeySize))
        return false;
    if (!GetSetValue(config, OT_KEY_SYMMETRIC_KEY_SIZE_MAX,
                     OT_DEFAULT_SYMMETRIC_KEY_SIZE_MAX,
                     sp_nSymmetricKeySizeMax))
        return false;
    if (!GetSetValue(config, OT_KEY_SYMMETRIC_IV_SIZE,
                     OT_DEFAULT_SYMMETRIC_IV_SIZE, sp_nSymmetricIvSize))
        return false;
    if (!GetSetValue(config, OT_KEY_SYMMETRIC_BUFFER_SIZE,
                     OT_DEFAULT_SYMMETRIC_BUFFER_SIZE, sp_nSymmetricBufferSize))
        return false;
    if (!GetSetValue(config, OT_KEY_PUBLIC_KEYSIZE, OT_DEFAULT_PUBLIC_KEYSIZE,
                     sp_nPublicKeysize))
        return false;
    if (!GetSetValue(config, OT_KEY_PUBLIC_KEYSIZE_MAX,
                     OT_DEFAULT_PUBLIC_KEYSIZE_MAX, sp_nPublicKeysizeMax))
        return false;

    if (!config.Save()) return false;

    config.Reset();

    return true;
}

bool OTCryptoConfig::GetSetValue(OTSettings& config, std::string strKeyName,
                                 int32_t nDefaultValue,
                                 const int32_t*& out_nValue)

{
    if (strKeyName.empty()) return false;
    if (3 > strKeyName.size()) return false;

    {
        bool bIsNew = false;
        int64_t nValue = 0;
        config.CheckSet_long("crypto", strKeyName, nDefaultValue, nValue,
                             bIsNew);

        if (nullptr != out_nValue) {
            delete out_nValue;
            out_nValue = nullptr;
        }

        out_nValue =
            new int32_t(bIsNew ? nDefaultValue : static_cast<int32_t>(nValue));
    }

    return true;
}

const int32_t& OTCryptoConfig::GetValue(const int32_t*& pValue)
{
    if (nullptr == pValue) {
        if (!GetSetAll()) OT_FAIL;
    }
    if (nullptr == pValue) {
        OT_FAIL;
    }
    return *pValue;
}

uint32_t OTCryptoConfig::IterationCount()
{
    return GetValue(sp_nIterationCount);
}
uint32_t OTCryptoConfig::SymmetricSaltSize()
{
    return GetValue(sp_nSymmetricSaltSize);
}
uint32_t OTCryptoConfig::SymmetricKeySize()
{
    return GetValue(sp_nSymmetricKeySize);
}
uint32_t OTCryptoConfig::SymmetricKeySizeMax()
{
    return GetValue(sp_nSymmetricKeySizeMax);
}
uint32_t OTCryptoConfig::SymmetricIvSize()
{
    return GetValue(sp_nSymmetricIvSize);
}
uint32_t OTCryptoConfig::SymmetricBufferSize()
{
    return GetValue(sp_nSymmetricBufferSize);
}
uint32_t OTCryptoConfig::PublicKeysize()
{
    return GetValue(sp_nPublicKeysize);
}
uint32_t OTCryptoConfig::PublicKeysizeMax()
{
    return GetValue(sp_nPublicKeysizeMax);
}

// static
int32_t OTCrypto::s_nCount =
    0; // Instance count, should never exceed 1. (At this point, anyway.)

// Currently called by OTLog::OT_Init();

void OTCrypto::Init() const
{
    // This is only supposed to happen once per run.
    //
    if (0 == OTCrypto::s_nCount) {
        ++(OTCrypto::s_nCount);

        otWarn << "OT_Init: Setting up rlimits, and crypto library...\n";

// Here is a security measure intended to make it more difficult to capture a
// core
// dump. (Not used in debug mode, obviously.)
//
#if !defined(PREDEF_MODE_DEBUG) && defined(PREDEF_PLATFORM_UNIX)
        struct rlimit rlim;
        getrlimit(RLIMIT_CORE, &rlim);
        rlim.rlim_max = rlim.rlim_cur = 0;
        if (setrlimit(RLIMIT_CORE, &rlim)) {
            OT_FAIL_MSG("OTCrypto::Init: ASSERT: setrlimit failed. (Used for "
                        "preventing core dumps.)\n");
        }
#endif

        Init_Override();
    }
    else
        otErr << "OTCrypto::Init: ERROR: Somehow this erroneously got called "
                 "more than once! (Doing nothing.)\n";
}

// Currently called by OTLog::OT_Cleanup();

void OTCrypto::Cleanup() const
{
    // This is only supposed to happen once per run.
    //
    if (1 == OTCrypto::s_nCount) {
        --(OTCrypto::s_nCount);

        // Any crypto-related cleanup code NOT specific to OpenSSL (which is
        // handled in OTCrypto_OpenSSL, a subclass) would go here.
        //

        Cleanup_Override();
    }
    else
        otErr << "OTCrypto::Cleanup: ERROR: Somehow this erroneously got "
                 "called more than once! (Doing nothing.)\n";
}

// virtual (Should never get called.)
void OTCrypto::Init_Override() const
{
    otErr << "OTCrypto::Init_Override: ERROR: This function should NEVER be "
             "called (you should be overriding it...)\n";
}

// virtual (Should never get called.)
void OTCrypto::Cleanup_Override() const
{
    otErr << "OTCrypto::Cleanup_Override: ERROR: This function should NEVER be "
             "called (you should be overriding it...)\n";
}

} // namespace opentxs
