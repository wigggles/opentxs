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

#include <opentxs/core/crypto/CryptoEngine.hpp>
#include <opentxs/core/crypto/CryptoUtil.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>

namespace opentxs
{

Libsecp256k1::Libsecp256k1(CryptoUtil& ssl)
    : Crypto(),
    context_(secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY)),
    ssl_(ssl)
{
    OT_ASSERT_MSG(nullptr != context_, "Libsecp256k1::Libsecp256k1: secp256k1_context_create failed.");
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

bool Libsecp256k1::secp256k1_privkey_tweak_add(
    uint8_t key [32],
    const uint8_t tweak [32]) const
{
    if (nullptr != context_) {
        return secp256k1_ec_privkey_tweak_add(context_, key, tweak);
    } else {
        return false;
    }
}

bool Libsecp256k1::secp256k1_pubkey_create(
    secp256k1_pubkey_t& pubkey,
    const OTPassword& privkey) const
{
    if (nullptr != context_) {
        return secp256k1_ec_pubkey_create(context_, &pubkey, static_cast<const unsigned char*>(privkey.getMemory()));
    }

    return false;
}

bool Libsecp256k1::secp256k1_pubkey_serialize(
        OTPassword& serializedPubkey,
        const secp256k1_pubkey_t& pubkey) const
{
    if (nullptr != context_) {
        uint8_t serializedOutput [65] {};
        int serializedSize = 0;

        bool serialized = secp256k1_ec_pubkey_serialize(context_, serializedOutput, &serializedSize, &pubkey, false);

        if (serialized) {
            serializedPubkey.setMemory(serializedOutput, serializedSize);
            return serialized;
        }

    }

    return false;
}

Libsecp256k1::~Libsecp256k1()
{
    OT_ASSERT_MSG(nullptr != context_, "Libsecp256k1::~Libsecp256k1: context_ should never be nullptr, yet it was.")
    secp256k1_context_destroy(context_);
    context_ = nullptr;
}

void Libsecp256k1::Init_Override() const
{
    static bool bNotAlreadyInitialized = true;
    OT_ASSERT_MSG(bNotAlreadyInitialized, "Libsecp256k1::Init_Override: Tried to initialize twice.");
    bNotAlreadyInitialized = false;
    // --------------------------------
    uint8_t randomSeed [32]{};
    ssl_.RandomizeMemory(randomSeed, 32);

    OT_ASSERT_MSG(nullptr != context_, "Libsecp256k1::Libsecp256k1: secp256k1_context_create failed.");

    int __attribute__((unused)) randomize = secp256k1_context_randomize(context_,
                                                                        randomSeed);
}

void Libsecp256k1::Cleanup_Override() const
{
}

} // namespace opentxs
