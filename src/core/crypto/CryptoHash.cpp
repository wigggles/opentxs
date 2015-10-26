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

#include <opentxs/core/crypto/CryptoHash.hpp>

#include <opentxs/core/OTData.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>

namespace opentxs
{

bool CryptoHash::Digest(
    const HashType hashType,
    const String& data,
    OTData& digest)
{
    OTData plaintext(data.Get(), data.GetLength());

    return Digest(hashType, plaintext, digest);
}

bool CryptoHash::HMAC(
        const CryptoHash::HashType hashType,
        const OTPassword& inputKey,
        const String& inputData,
        OTPassword& outputDigest) const
{
    OTData convertedData(inputData.Get(), inputData.GetLength());

    return HMAC(hashType, inputKey, convertedData, outputDigest);
}

CryptoHash::HashType CryptoHash::StringToHashType(const String& inputString)
{
    if (inputString.Compare("HASH256"))
        return CryptoHash::HASH256;
    else if (inputString.Compare("HASH160"))
        return CryptoHash::HASH160;
    else if (inputString.Compare("SHA1"))
        return CryptoHash::SHA1;
    else if (inputString.Compare("SHA224"))
        return CryptoHash::SHA224;
    else if (inputString.Compare("SHA256"))
        return CryptoHash::SHA256;
    else if (inputString.Compare("SHA384"))
        return CryptoHash::SHA384;
    else if (inputString.Compare("SHA512"))
        return CryptoHash::SHA512;
    return CryptoHash::ERROR;
}

String CryptoHash::HashTypeToString(const CryptoHash::HashType hashType)

{
    String hashTypeString;

    switch (hashType) {
        case CryptoHash::HASH256 :
            hashTypeString = "HASH256";
            break;
        case CryptoHash::HASH160 :
            hashTypeString = "HASH160";
            break;
        case CryptoHash::SHA1 :
            hashTypeString = "SHA1";
            break;
        case CryptoHash::SHA224 :
            hashTypeString = "SHA224";
            break;
        case CryptoHash::SHA256 :
            hashTypeString = "SHA256";
            break;
        case CryptoHash::SHA384 :
            hashTypeString = "SHA384";
            break;
        case CryptoHash::SHA512 :
            hashTypeString = "SHA512";
            break;
        default :
            hashTypeString = "ERROR";
    }
    return hashTypeString;
}

} // namespace opentxs
