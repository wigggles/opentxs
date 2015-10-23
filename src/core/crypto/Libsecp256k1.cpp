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

#include <opentxs/core/FormattedKey.hpp>
#include <opentxs/core/crypto/BitcoinCrypto.hpp>
#include <opentxs/core/crypto/CryptoEngine.hpp>
#include <opentxs/core/crypto/CryptoUtil.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTSignature.hpp>
#include <opentxs/core/crypto/AsymmetricKeySecp256k1.hpp>

#include <vector>

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
    const String& strContractUnsigned,
    const OTAsymmetricKey& theKey,
    OTSignature& theSignature, // output
    CryptoHash::HashType hashType,
    __attribute__((unused)) const OTPasswordData* pPWData)
{
    String hash;
    OTData plaintext(strContractUnsigned.Get(), strContractUnsigned.GetLength());
    bool haveDigest = CryptoEngine::Instance().Hash().Hash(hashType, plaintext, hash);

    if (haveDigest) {
        OTPassword privKey;
        bool havePrivateKey = AsymmetricKeyToECDSAPrivkey(theKey, privKey);

        if (havePrivateKey) {
            secp256k1_ecdsa_signature_t ecdsaSignature;

            bool signatureCreated = secp256k1_ecdsa_sign(
                context_,
                &ecdsaSignature,
                reinterpret_cast<const unsigned char*>(hash.Get()),
                reinterpret_cast<const unsigned char*>(privKey.getMemory()),
                nullptr,
                nullptr);

            if (signatureCreated) {
                bool signatureSet = ECDSASignatureToOTSignature(ecdsaSignature, theSignature);
                return signatureSet;
            }
        }
    }
    return false;
}

bool Libsecp256k1::VerifySignature(
    const String& strContractToVerify,
    const OTAsymmetricKey& theKey,
    const OTSignature& theSignature,
    const CryptoHash::HashType hashType,
    __attribute__((unused)) const OTPasswordData* pPWData
    ) const
{
    String hash;
    OTData plaintext(strContractToVerify.Get(), strContractToVerify.GetLength());
    bool haveDigest = CryptoEngine::Instance().Hash().Hash(hashType, plaintext, hash);

    if (haveDigest) {
        secp256k1_pubkey_t ecdsaPubkey;
        bool havePublicKey = AsymmetricKeyToECDSAPubkey(theKey, ecdsaPubkey);

        if (havePublicKey) {
            secp256k1_ecdsa_signature_t ecdsaSignature;

            bool haveSignature = OTSignatureToECDSASignature(theSignature, ecdsaSignature);

            if (haveSignature) {
                bool signatureVerified = secp256k1_ecdsa_verify(
                    context_,
                    &ecdsaSignature,
                    reinterpret_cast<const unsigned char*>(hash.Get()),
                    &ecdsaPubkey);

                return signatureVerified;
            }
        }
    }
    return false;
}

bool Libsecp256k1::OTSignatureToECDSASignature(
    const OTSignature& inSignature,
    secp256k1_ecdsa_signature_t& outSignature) const
{
    OTData signature;

    bool hasSignature = inSignature.GetData(signature);

    if (hasSignature) {
        const uint8_t* sigStart = static_cast<const uint8_t*>(signature.GetPointer());

        if (nullptr != sigStart) {

            if (sizeof(secp256k1_ecdsa_signature_t) == signature.GetSize()) {
                secp256k1_ecdsa_signature_t ecdsaSignature;

                for(uint32_t i=0; i < signature.GetSize(); i++) {
                    ecdsaSignature.data[i] = *(sigStart + i);
                }

                outSignature = ecdsaSignature;

                return true;
            }
        }
    }
    return false;
}

bool Libsecp256k1::ECDSASignatureToOTSignature(
    const secp256k1_ecdsa_signature_t& inSignature,
    OTSignature& outSignature) const
{
    OTData signature;

    signature.Assign(inSignature.data, sizeof(secp256k1_ecdsa_signature_t));
    bool signatureSet = outSignature.SetData(signature);

    return signatureSet;
}

bool Libsecp256k1::AsymmetricKeyToECDSAPubkey(
        const OTAsymmetricKey& asymmetricKey,
        secp256k1_pubkey_t& pubkey) const
{
    String encodedPubkey;
    bool havePublicKey = asymmetricKey.GetPublicKey(encodedPubkey);

    if (havePublicKey) {
        std::vector<unsigned char> decodedPublicKey;
        bool pubkeydecoded = DecodeBase58Check(encodedPubkey.Get(), decodedPublicKey);

        if (pubkeydecoded) {
            OTData serializedPubkey(decodedPublicKey);
            secp256k1_pubkey_t parsedPubkey;

            bool pubkeyParsed = secp256k1_ec_pubkey_parse(
                context_,
                &parsedPubkey,
                reinterpret_cast<const unsigned char*>(serializedPubkey.GetPointer()),
                serializedPubkey.GetSize());

            if (pubkeyParsed) {
                pubkey = parsedPubkey;
                return true;
            }
        }
    }
    return false;
}

bool Libsecp256k1::ECDSAPubkeyToAsymmetricKey(
        const secp256k1_pubkey_t& pubkey,
        OTAsymmetricKey& asymmetricKey) const
{
    OTData serializedPubkey;

    bool keySerialized = secp256k1_pubkey_serialize(serializedPubkey, pubkey);

    if (keySerialized) {
        const uint8_t* keyStart = static_cast<const uint8_t*>(serializedPubkey.GetPointer());
        const uint8_t* keyEnd = keyStart + serializedPubkey.GetSize();

        FormattedKey encodedPublicKey(EncodeBase58Check(keyStart, keyEnd));

        return asymmetricKey.SetPublicKey(encodedPublicKey);
    }
    return false;
}

bool Libsecp256k1::AsymmetricKeyToECDSAPrivkey(
        const OTAsymmetricKey& asymmetricKey,
        OTPassword& privkey) const
{
    FormattedKey encodedPrivkey;
    bool havePrivateKey = asymmetricKey.GetPrivateKey(encodedPrivkey);

    if (havePrivateKey) {
        std::vector<unsigned char> decodedPrivateKey;
        bool privkeydecoded = DecodeBase58Check(encodedPrivkey.Get(), decodedPrivateKey);

        if (privkeydecoded) {
            OTData serializedPubkey(decodedPrivateKey);

            return privkey.setMemory(serializedPubkey);
        }
    }
    return false;
}

bool Libsecp256k1::ECDSAPrivkeyToAsymmetricKey(
        const OTPassword& privkey,
        OTAsymmetricKey& asymmetricKey) const
{
    const uint8_t* keyStart = static_cast<const uint8_t*>(privkey.getMemory());
    const uint8_t* keyEnd = keyStart + privkey.getMemorySize();

    FormattedKey encodedPrivateKey(EncodeBase58Check(keyStart, keyEnd));

    return asymmetricKey.SetPrivateKey(encodedPrivateKey);
}

bool Libsecp256k1::secp256k1_privkey_tweak_add(
    uint8_t key [PrivateKeySize],
    const uint8_t tweak [PrivateKeySize]) const
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
        OTData& serializedPubkey,
        const secp256k1_pubkey_t& pubkey) const
{
    if (nullptr != context_) {
        uint8_t serializedOutput [65] {};
        int serializedSize = 0;

        bool serialized = secp256k1_ec_pubkey_serialize(context_, serializedOutput, &serializedSize, &pubkey, false);

        if (serialized) {
            serializedPubkey.Assign(serializedOutput, serializedSize);
            return serialized;
        }
    }

    return false;
}

bool Libsecp256k1::secp256k1_pubkey_parse(
        secp256k1_pubkey_t& pubkey,
        const OTPassword& serializedPubkey) const
{
    if (nullptr != context_) {

        const uint8_t* inputStart = static_cast<const uint8_t*>(serializedPubkey.getMemory());

        bool parsed = secp256k1_ec_pubkey_parse(context_, &pubkey, inputStart, serializedPubkey.getMemorySize());

        return parsed;
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
