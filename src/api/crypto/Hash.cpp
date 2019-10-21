// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/library/EncodingProvider.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"
#include "opentxs/crypto/library/Ripemd160.hpp"
#include "opentxs/network/zeromq/Frame.hpp"

#include "siphash/src/siphash.h"
#include "smhasher/src/MurmurHash3.h"

#include "Hash.hpp"

#define OT_METHOD "opentxs::api::crypto::implementation::Hash::"

namespace opentxs
{
api::crypto::Hash* Factory::Hash(
    const api::crypto::Encode& encode,
    const crypto::HashingProvider& ssl,
    const crypto::HashingProvider& sodium,
    const crypto::Ripemd160& ripe)
{
    return new api::crypto::implementation::Hash(encode, ssl, sodium, ripe);
}
}  // namespace opentxs

namespace opentxs::api::crypto::implementation
{
Hash::Hash(
    const api::crypto::Encode& encode,
    const opentxs::crypto::HashingProvider& ssl,
    const opentxs::crypto::HashingProvider& sodium,
    const opentxs::crypto::Ripemd160& ripe) noexcept
    : encode_(encode)
    , ssl_(ssl)
    , sodium_(sodium)
    , ripe_(ripe)
{
}

bool Hash::Allocate(const proto::HashType hashType, OTPassword& input) noexcept
{
    return input.randomizeMemory(
        opentxs::crypto::HashingProvider::HashSize(hashType));
}

bool Hash::Allocate(const proto::HashType hashType, Data& input) noexcept
{
    return input.Randomize(
        opentxs::crypto::HashingProvider::HashSize(hashType));
}

bool Hash::Digest(
    const proto::HashType hashType,
    const OTPassword& data,
    OTPassword& digest) const noexcept
{
    if (false == Allocate(hashType, digest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

        return false;
    }

    if (false == data.isMemory()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong OTPassword mode.").Flush();

        return false;
    }

    return Digest(
        hashType,
        static_cast<const std::uint8_t*>(data.getMemory()),
        data.getMemorySize(),
        static_cast<std::uint8_t*>(digest.getMemoryWritable()));
}

bool Hash::Digest(
    const proto::HashType hashType,
    const Data& data,
    Data& digest) const noexcept
{
    if (false == Allocate(hashType, digest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

        return false;
    }

    return Digest(
        hashType,
        static_cast<const std::uint8_t*>(data.data()),
        data.size(),
        static_cast<std::uint8_t*>(digest.data()));
}

bool Hash::Digest(
    const proto::HashType hashType,
    const String& data,
    Data& digest) const noexcept
{
    if (false == Allocate(hashType, digest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

        return false;
    }

    return Digest(
        hashType,
        reinterpret_cast<const std::uint8_t*>(data.Get()),
        data.GetLength(),
        static_cast<std::uint8_t*>(digest.data()));
}

bool Hash::Digest(
    const proto::HashType hashType,
    const opentxs::network::zeromq::Frame& data,
    Data& digest) const noexcept
{
    if (false == Allocate(hashType, digest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

        return false;
    }

    return Digest(
        hashType,
        static_cast<const std::uint8_t*>(data.data()),
        data.size(),
        static_cast<std::uint8_t*>(digest.data()));
}

bool Hash::Digest(
    const proto::HashType hashType,
    const std::string& data,
    Data& digest) const noexcept
{
    if (false == Allocate(hashType, digest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

        return false;
    }

    return Digest(
        hashType,
        reinterpret_cast<const std::uint8_t*>(data.c_str()),
        data.size(),
        static_cast<std::uint8_t*>(digest.data()));
}

bool Hash::Digest(
    const proto::HashType hashType,
    const void* data,
    const std::size_t size,
    Data& digest) const noexcept
{
    if (false == Allocate(hashType, digest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

        return false;
    }

    return Digest(
        hashType,
        static_cast<const std::uint8_t*>(data),
        size,
        static_cast<std::uint8_t*>(digest.data()));
}

bool Hash::Digest(
    const std::uint32_t type,
    const std::string& data,
    std::string& encodedDigest) const noexcept
{
    proto::HashType hashType = static_cast<proto::HashType>(type);
    auto result = Data::Factory();

    if (false == Allocate(hashType, result)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

        return false;
    }

    const bool success = Digest(
        hashType,
        reinterpret_cast<const std::uint8_t*>(data.c_str()),
        data.size(),
        static_cast<std::uint8_t*>(const_cast<void*>(result->data())));

    if (success) { encodedDigest.assign(encode_.IdentifierEncode(result)); }

    return success;
}

bool Hash::Digest(
    const proto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    std::uint8_t* output) const noexcept
{
    switch (hashType) {
        case (proto::HASHTYPE_SHA256):
        case (proto::HASHTYPE_SHA512): {
            return SHA2().Digest(hashType, input, inputSize, output);
        }
        case (proto::HASHTYPE_BLAKE2B160):
        case (proto::HASHTYPE_BLAKE2B256):
        case (proto::HASHTYPE_BLAKE2B512): {
            return Sodium().Digest(hashType, input, inputSize, output);
        }
        case (proto::HASHTYPE_RIMEMD160): {
            return ripe_.RIPEMD160(input, inputSize, output);
        }
        default: {
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported hash type.").Flush();

    return false;
}

bool Hash::HMAC(
    const proto::HashType hashType,
    const OTPassword& key,
    const Data& data,
    OTPassword& digest) const noexcept
{
    if (false == Allocate(hashType, digest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

        return false;
    }

    if (false == key.isMemory()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong OTPassword mode.").Flush();

        return false;
    }

    return HMAC(
        hashType,
        static_cast<const std::uint8_t*>(data.data()),
        data.size(),
        static_cast<const std::uint8_t*>(key.getMemory()),
        key.getMemorySize(),
        static_cast<std::uint8_t*>(digest.getMemoryWritable()));
}

bool Hash::HMAC(
    const proto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    const std::uint8_t* key,
    const size_t keySize,
    std::uint8_t* output) const noexcept
{
    switch (hashType) {
        case (proto::HASHTYPE_SHA256):
        case (proto::HASHTYPE_SHA512): {
            return SHA2().HMAC(
                hashType, input, inputSize, key, keySize, output);
        }
        case (proto::HASHTYPE_BLAKE2B160):
        case (proto::HASHTYPE_BLAKE2B256):
        case (proto::HASHTYPE_BLAKE2B512): {
            return Sodium().HMAC(
                hashType, input, inputSize, key, keySize, output);
        }
        default: {
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported hash type.").Flush();

    return false;
}

void Hash::MurmurHash3_32(
    const std::uint32_t& key,
    const Data& data,
    std::uint32_t& output) const noexcept
{
    MurmurHash3_x86_32(data.data(), data.size(), key, &output);
}

bool Hash::SipHash(
    const OTPassword& key,
    const Data& data,
    std::uint64_t& output,
    const int c,
    const int d) const noexcept
{
    const bool validKey =
        16 == (key.isMemory() ? key.getMemorySize() : key.getPasswordSize());

    if (false == validKey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid key size").Flush();

        return false;
    }

    if (1 > c) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid c").Flush();

        return false;
    }

    if (1 > d) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid d").Flush();

        return false;
    }

    ::SipHash hasher{const_cast<char*>(static_cast<const char*>(
                         key.isMemory() ? key.getMemory() : key.getPassword())),
                     c,
                     d};

    for (const auto& byte : data) {
        hasher.update(std::to_integer<char>(byte));
    }

    output = hasher.digest();

    return true;
}
}  // namespace opentxs::api::crypto::implementation
