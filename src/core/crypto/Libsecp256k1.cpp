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
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/core/crypto/Libsecp256k1.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/app/App.hpp"
#include "opentxs/core/crypto/AsymmetricKeySecp256k1.hpp"
#include "opentxs/core/crypto/Crypto.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHashEngine.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/CryptoUtil.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#if OT_CRYPTO_USING_TREZOR
#include "opentxs/core/crypto/TrezorCrypto.hpp"
#endif
#include "opentxs/core/util/Assert.hpp"

#include <stdint.h>
#include <ostream>

namespace opentxs
{

Libsecp256k1::Libsecp256k1(CryptoUtil& ssl, Ecdsa& ecdsa)
    : Crypto()
    , context_(secp256k1_context_create(
        SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY))
    , ecdsa_(ecdsa)
    , ssl_(ssl)
{
    OT_ASSERT_MSG(nullptr != context_, "secp256k1_context_create failed.");
}

bool Libsecp256k1::RandomKeypair(
    OTPassword& privateKey,
    OTData& publicKey) const
{
    if (nullptr == context_) { return false; }

    bool validPrivkey = false;
    std::uint8_t candidateKey [PrivateKeySize]{};
    std::uint8_t nullKey [PrivateKeySize]{};
    std::uint8_t counter = 0;

    while (!validPrivkey) {
        privateKey.randomizeMemory_uint8(candidateKey, sizeof(candidateKey));
        // We add the random key to a zero value key because
        // secp256k1_privkey_tweak_add checks the result to make sure it's in
        // the correct range for secp256k1.
        //
        // This loop should almost always run exactly one time (about 1/(2^128)
        // chance of randomly generating an invalid key thus requiring a second
        // attempt)
        validPrivkey = secp256k1_ec_privkey_tweak_add(
            context_, candidateKey, nullKey);

        OT_ASSERT(3 > ++counter);
    }
    privateKey.setMemory(candidateKey, sizeof(candidateKey));

    return ScalarBaseMultiply(privateKey, publicKey);
}

bool Libsecp256k1::Sign(
    const OTData& plaintext,
    const OTAsymmetricKey& theKey,
    const proto::HashType hashType,
    OTData& signature, // output
    const OTPasswordData* pPWData,
    const OTPassword* exportPassword) const
{
    OTData hash;
    bool haveDigest = App::Me().Crypto().Hash().Digest(hashType, plaintext, hash);

    if (haveDigest) {
        OTPassword privKey;
        bool havePrivateKey;

        if (nullptr == pPWData) {
            OTPasswordData passwordData("Libsecp256k1::SignContract(): Please enter your password to sign this document.");
            havePrivateKey = AsymmetricKeyToECPrivatekey(theKey, passwordData, privKey, exportPassword);
        } else {
            havePrivateKey = AsymmetricKeyToECPrivatekey(theKey, *pPWData, privKey, exportPassword);
        }

        if (havePrivateKey) {
            secp256k1_ecdsa_signature ecdsaSignature;

            bool signatureCreated = secp256k1_ecdsa_sign(
                context_,
                &ecdsaSignature,
                reinterpret_cast<const unsigned char*>(hash.GetPointer()),
                reinterpret_cast<const unsigned char*>(privKey.getMemory()),
                nullptr,
                nullptr);

            if (signatureCreated) {
                signature.Assign(ecdsaSignature.data, sizeof(secp256k1_ecdsa_signature));
                return true;
            } else {
                    otErr << __FUNCTION__ << ": "
                    << "Call to secp256k1_ecdsa_sign() failed.\n";

                    return false;
            }
        } else {
                otErr << __FUNCTION__ << ": "
                << "Can not extract ecdsa private key from OTAsymmetricKey.\n";

                return false;
        }
    } else {
        otErr << __FUNCTION__ << ": "
        << "Failed to obtain the contract hash.\n";

        return false;
    }
}

bool Libsecp256k1::Verify(
    const OTData& plaintext,
    const OTAsymmetricKey& theKey,
    const OTData& signature,
    const proto::HashType hashType,
    __attribute__((unused)) const OTPasswordData* pPWData) const
{
    OTData hash;
    bool haveDigest = App::Me().Crypto().Hash().Digest(hashType, plaintext, hash);

    if (!haveDigest) { return false; }

    OTData ecdsaPubkey;
    const bool havePublicKey = AsymmetricKeyToECPubkey(theKey, ecdsaPubkey);

    if (!havePublicKey) { return false; }

    secp256k1_pubkey point;
    const bool pubkeyParsed = ParsePublicKey(ecdsaPubkey, point);

    if (!pubkeyParsed) { return false; }

    secp256k1_ecdsa_signature ecdsaSignature;
    const bool haveSignature = OTDataToECSignature(signature, ecdsaSignature);

    if (!haveSignature) { return false; }

    return secp256k1_ecdsa_verify(
        context_,
        &ecdsaSignature,
        reinterpret_cast<const unsigned char*>(hash.GetPointer()),
        &point);
}

bool Libsecp256k1::OTDataToECSignature(
    const OTData& inSignature,
    secp256k1_ecdsa_signature& outSignature) const
{
    const uint8_t* sigStart = static_cast<const uint8_t*>(inSignature.GetPointer());

    if (nullptr != sigStart) {

        if (sizeof(secp256k1_ecdsa_signature) == inSignature.GetSize()) {
            secp256k1_ecdsa_signature ecdsaSignature;

            for(uint32_t i=0; i < inSignature.GetSize(); i++) {
                ecdsaSignature.data[i] = *(sigStart + i);
            }

            outSignature = ecdsaSignature;

            return true;
        }
    }
    return false;
}

bool Libsecp256k1::ECDH(
    const OTData& publicKey,
    const OTPassword& privateKey,
    OTPassword& secret) const
{
#if OT_CRYPTO_USING_TREZOR
    return static_cast<TrezorCrypto&>(ecdsa_).ECDH(
        publicKey, privateKey, secret);
#else
    return false;
#endif
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

bool Libsecp256k1::ParsePublicKey(
    const OTData& input,
    secp256k1_pubkey& output) const
{
    if (nullptr == context_) { return false; }

    return secp256k1_ec_pubkey_parse(
        context_,
        &output,
        reinterpret_cast<const unsigned char*>(input.GetPointer()),
        input.GetSize());
}

bool Libsecp256k1::ScalarBaseMultiply(
    const OTPassword& privateKey,
    OTData& publicKey) const
{
    if (nullptr == context_) { return false; }

    if (!privateKey.isMemory()) { return false; }

    secp256k1_pubkey key;

    const auto created = secp256k1_ec_pubkey_create(
        context_,
        &key,
        static_cast<const unsigned char*>(privateKey.getMemory()));

    if (1 != created) { return false; }

    unsigned char output[PublicKeySize]{};
    size_t outputSize = sizeof(output);

    const auto serialized = secp256k1_ec_pubkey_serialize(
        context_,
        output,
        &outputSize,
        &key,
        SECP256K1_EC_COMPRESSED);

    if (1 != serialized) { return false; }

    publicKey.Assign(output, outputSize);

    return true;
}

Libsecp256k1::~Libsecp256k1()
{
    if (nullptr != context_) {
        secp256k1_context_destroy(context_);
        context_ = nullptr;
    }
}
} // namespace opentxs
# endif
