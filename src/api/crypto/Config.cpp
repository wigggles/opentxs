// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/core/Log.hpp"

#include <ostream>
#include <string>

#include "Config.hpp"

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

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

namespace opentxs
{
api::crypto::Config* Factory::CryptoConfig(const api::Settings& settings)
{
    return new api::crypto::implementation::Config(settings);
}
}  // namespace opentxs

namespace opentxs::api::crypto::implementation
{
Config::Config(const api::Settings& settings)
    : config_(settings)
{
    GetSetAll();
}

bool Config::GetSetAll() const
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

    return config_.Save();
}

bool Config::GetSetValue(
    const std::string& strKeyName,
    const std::int32_t nDefaultValue,
    std::int32_t& out_nValue) const

{
    OT_ASSERT(false == strKeyName.empty())
    OT_ASSERT(2 < strKeyName.size())

    bool bIsNew{false};
    std::int64_t nValue{0};
    config_.CheckSet_long(
        String::Factory("crypto"),
        String::Factory(strKeyName),
        nDefaultValue,
        nValue,
        bIsNew);
    out_nValue = static_cast<std::int32_t>(nValue);

    return true;
}

std::uint32_t Config::IterationCount() const { return sp_nIterationCount; }
std::uint32_t Config::SymmetricSaltSize() const
{
    return sp_nSymmetricSaltSize;
}
std::uint32_t Config::SymmetricKeySize() const { return sp_nSymmetricKeySize; }
std::uint32_t Config::SymmetricKeySizeMax() const
{
    return sp_nSymmetricKeySizeMax;
}
std::uint32_t Config::SymmetricIvSize() const { return sp_nSymmetricIvSize; }
std::uint32_t Config::SymmetricBufferSize() const
{
    return sp_nSymmetricBufferSize;
}
std::uint32_t Config::PublicKeysize() const { return sp_nPublicKeysize; }
std::uint32_t Config::PublicKeysizeMax() const { return sp_nPublicKeysizeMax; }
}  // namespace opentxs::api::crypto::implementation
