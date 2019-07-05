// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CRYPTO_HASH_HPP
#define OPENTXS_API_CRYPTO_HASH_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <cstdint>
#include <string>

namespace opentxs
{
namespace api
{
namespace crypto
{

class Hash
{
public:
    EXPORT virtual bool Digest(
        const proto::HashType hashType,
        const OTPassword& data,
        OTPassword& digest) const noexcept = 0;
    EXPORT virtual bool Digest(
        const proto::HashType hashType,
        const Data& data,
        Data& digest) const noexcept = 0;
    EXPORT virtual bool Digest(
        const proto::HashType hashType,
        const String& data,
        Data& digest) const noexcept = 0;
    EXPORT virtual bool Digest(
        const proto::HashType hashType,
        const std::string& data,
        Data& digest) const noexcept = 0;
    EXPORT virtual bool Digest(
        const std::uint32_t type,
        const std::string& data,
        std::string& encodedDigest) const noexcept = 0;
    EXPORT virtual bool HMAC(
        const proto::HashType hashType,
        const OTPassword& key,
        const Data& data,
        OTPassword& digest) const noexcept = 0;
    EXPORT virtual void MurmurHash3_32(
        const std::uint32_t& key,
        const Data& data,
        std::uint32_t& output) const noexcept = 0;
    EXPORT virtual bool SipHash(
        const OTPassword& key,
        const Data& data,
        std::uint64_t& output,
        const int c = 2,
        const int d = 4) const noexcept = 0;

    EXPORT virtual ~Hash() = default;

protected:
    Hash() noexcept = default;

private:
    Hash(const Hash&) = delete;
    Hash(Hash&&) = delete;
    Hash& operator=(const Hash&) = delete;
    Hash& operator=(Hash&&) = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif
