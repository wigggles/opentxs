// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_HASHINGPROVIDER_HPP
#define OPENTXS_CRYPTO_LIBRARY_HASHINGPROVIDER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <cstdint>

namespace opentxs
{
namespace crypto
{
class HashingProvider
{

public:
    static proto::HashType StringToHashType(const String& inputString);
    static OTString HashTypeToString(const proto::HashType hashType);
    static std::size_t HashSize(const proto::HashType hashType);

    EXPORT virtual bool Digest(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const std::size_t inputSize,
        std::uint8_t* output) const = 0;
    EXPORT virtual bool HMAC(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* key,
        const std::size_t keySize,
        std::uint8_t* output) const = 0;

    EXPORT virtual ~HashingProvider() = default;

protected:
    HashingProvider() = default;

private:
    HashingProvider(const HashingProvider&) = delete;
    HashingProvider(HashingProvider&&) = delete;
    HashingProvider& operator=(const HashingProvider&) = delete;
    HashingProvider& operator=(HashingProvider&&) = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif
