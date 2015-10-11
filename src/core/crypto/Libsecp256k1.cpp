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

#include <opentxs/core/crypto/Libsecp256k1.hpp>

#include <opentxs/core/OTData.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>

namespace opentxs
{

Libsecp256k1::Libsecp256k1()
    : Crypto(),
    context_(secp256k1_context_create(SECP256K1_CONTEXT_SIGN & SECP256K1_CONTEXT_VERIFY))
{
    OTData randomSeed;
    randomSeed.Randomize(32);
    OTASCIIArmor randomSeedArmored(randomSeed);

    int __attribute__((unused)) randomize = secp256k1_context_randomize(
        context_,
        reinterpret_cast<const unsigned char*>(randomSeedArmored.Get()));
}

bool Libsecp256k1::Seal(
    __attribute__((unused)) mapOfAsymmetricKeys& RecipPubKeys,
    __attribute__((unused)) const String& theInput,
    __attribute__((unused)) OTData& dataOutput
    ) const
{
    return false;
}

bool Libsecp256k1::Open(
    __attribute__((unused)) OTData& dataInput,
    __attribute__((unused)) const Nym& theRecipient,
    __attribute__((unused)) String& theOutput,
    __attribute__((unused)) const OTPasswordData* pPWData
    ) const
{
    return false;
}

bool Libsecp256k1::SignContract(
    __attribute__((unused)) const String& strContractUnsigned,
    __attribute__((unused)) const OTAsymmetricKey& theKey,
    __attribute__((unused)) OTSignature& theSignature, // output
    __attribute__((unused)) const String& strHashType,
    __attribute__((unused)) const OTPasswordData* pPWData
    )
{
    return false;
}

bool Libsecp256k1::VerifySignature(
    __attribute__((unused)) const String& strContractToVerify,
    __attribute__((unused)) const OTAsymmetricKey& theKey,
    __attribute__((unused)) const OTSignature& theSignature,
    __attribute__((unused)) const String& strHashType,
    __attribute__((unused)) const OTPasswordData* pPWData
    ) const
{
    return false;
}

Libsecp256k1::~Libsecp256k1()
{
}

void Libsecp256k1::Init_Override() const
{
}

void Libsecp256k1::Cleanup_Override() const
{
    secp256k1_context_destroy(context_);
}

} // namespace opentxs
