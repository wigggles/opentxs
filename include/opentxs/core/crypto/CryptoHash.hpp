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

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOHASH_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOHASH_HPP

#include <opentxs-proto/verify/VerifyCredentials.hpp>

namespace opentxs
{

class OTData;
class OTPassword;
class String;

class CryptoHash
{
protected:
    CryptoHash() = default;

public:
    enum HashType {
      ERROR = proto::HASHTYPE_ERROR,
      NONE = proto::HASHTYPE_NONE,
      HASH256 = proto::HASHTYPE_BTC160,
      HASH160 = proto::HASHTYPE_BTC256,
      SHA224 = proto::HASHTYPE_SHA224,
      SHA256 = proto::HASHTYPE_SHA256,
      SHA384 = proto::HASHTYPE_SHA384,
      SHA512 = proto::HASHTYPE_SHA512
    };

    virtual ~CryptoHash() = default;
    virtual bool Digest(
        const HashType hashType,
        const OTPassword& data,
        OTPassword& digest) const = 0;
    virtual bool Digest(
        const HashType hashType,
        const OTData& data,
        OTData& digest) const = 0;
    virtual bool HMAC(
        const CryptoHash::HashType hashType,
        const OTPassword& inputKey,
        const OTData& inputData,
        OTPassword& outputDigest) const = 0;
    bool Digest(
        const HashType hashType,
        const String& data,
        OTData& digest);
    bool Digest(
        const uint32_t type,
        const std::string& data,
        std::string& encodedDigest);
    bool HMAC(
        const CryptoHash::HashType hashType,
        const OTPassword& inputKey,
        const String& inputData,
        OTPassword& outputDigest) const;

    static HashType StringToHashType(const String& inputString);
    static String HashTypeToString(const HashType hashType);
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_CRYPTOHASH_HPP
