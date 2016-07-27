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

#include "opentxs/core/Proto.hpp"

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
    virtual ~CryptoHash() = default;
    virtual bool Digest(
        const proto::HashType hashType,
        const OTPassword& data,
        OTPassword& digest) const = 0;
    virtual bool Digest(
        const proto::HashType hashType,
        const OTData& data,
        OTData& digest) const = 0;
    virtual bool HMAC(
        const proto::HashType hashType,
        const OTPassword& inputKey,
        const OTData& inputData,
        OTPassword& outputDigest) const = 0;
    bool Digest(
        const proto::HashType hashType,
        const String& data,
        OTData& digest);
    bool Digest(
        const uint32_t type,
        const std::string& data,
        std::string& encodedDigest);
    bool HMAC(
        const proto::HashType hashType,
        const OTPassword& inputKey,
        const String& inputData,
        OTPassword& outputDigest) const;

    static proto::HashType StringToHashType(const String& inputString);
    static String HashTypeToString(const proto::HashType hashType);
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_CRYPTOHASH_HPP
