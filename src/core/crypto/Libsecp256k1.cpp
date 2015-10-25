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
#include <opentxs/core/Log.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/core/crypto/BitcoinCrypto.hpp>
#include <opentxs/core/crypto/CryptoEngine.hpp>
#include <opentxs/core/crypto/CryptoUtil.hpp>
#include <opentxs/core/crypto/Letter.hpp>
#include <opentxs/core/crypto/NymParameters.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>
#include <opentxs/core/crypto/OTKeypair.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTSymmetricKey.hpp>
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
    mapOfAsymmetricKeys& RecipPubKeys,
    const String& theInput,
    OTData& dataOutput
    ) const
{
    mapOfAsymmetricKeys::iterator it = RecipPubKeys.begin();
    OTAsymmetricKey* recipient = it->second;

    OTKeypair ephemeralKeypair(OTAsymmetricKey::SECP256K1);
    std::shared_ptr<NymParameters> pKeyData;
    pKeyData = std::make_shared<NymParameters>(
        NymParameters::SECP256K1,
        Credential::SECP256K1_PUBKEY);
    ephemeralKeypair.MakeNewKeypair(pKeyData);

    FormattedKey ephemeralPubkey;
    const OTAsymmetricKey& ephemeralPrivkey = ephemeralKeypair.GetPrivateKey();

    OT_ASSERT_MSG(ECDHDefaultHMACSize>=CryptoConfig::SymmetricIvSize(), "Pick a larger HMAC. This one is too small.\n");

    String nonce = Nonce(ECDHDefaultHMACSize);
    ephemeralKeypair.GetPublicKey(ephemeralPubkey);

    std::unique_ptr<OTPassword> ECDHSecret(CryptoEngine::Instance().AES().InstantiateBinarySecret());
    bool haveECDH = ECDH(*recipient, ephemeralPrivkey, *ECDHSecret);

    if (haveECDH) {

        OTPassword sharedSecret;
        CryptoEngine::Instance().Hash().HMAC(ECDHDefaultHMAC, *ECDHSecret, nonce, sharedSecret);

        OTPassword sessionKey(sharedSecret);

        OTData nonceHash;
        CryptoEngine::Instance().Hash().Digest(ECDHDefaultHMAC, nonce, nonceHash);

        OTPassword truncatedSessionKey(sessionKey.getMemory(), CryptoConfig::SymmetricKeySize());
        OTData truncatedNonceHash(nonceHash.GetPointer(), CryptoConfig::SymmetricIvSize());

        OTData ciphertext;
        bool encrypted = CryptoEngine::Instance().AES().Encrypt(
            truncatedSessionKey,
            theInput.Get(),
            theInput.GetLength(),
            truncatedNonceHash,
            ciphertext
        );

        if (encrypted) {
            OTASCIIArmor encodedCiphertext(ciphertext);

            Letter theLetter(
                ephemeralPubkey,
                CryptoHash::HashTypeToString(ECDHDefaultHMAC),
                nonce,
                "",
                encodedCiphertext
            );

            String output;
            theLetter.UpdateContents();
            theLetter.SaveContents(output);

            OTASCIIArmor armoredOutput(output);
            OTData finishedOutput(armoredOutput);
            dataOutput.Assign(finishedOutput);

            return true;
        } else {
            otErr << "Libsecp256k1::" << __FUNCTION__ << ": Encryption failed.\n";
            return false;
        }
    } else {
        otErr << "Libsecp256k1::" << __FUNCTION__ << ": ECDH shared secret negotiation failed.\n";
        return false;
    }
}

bool Libsecp256k1::Open(
    OTData& dataInput,
    __attribute__((unused)) const Nym& theRecipient,
    String& theOutput,
    __attribute__((unused)) const OTPasswordData* pPWData
    ) const
{
    OTASCIIArmor armoredInput(dataInput);
    String decodedInput;
    OTData decodedCiphertext;
    String examplePassword("this is an example password");
    OTData passwordHash;
    OTPassword sessionKey;
    OTData nonceHash;
    OTData plaintext;

    bool haveDecodedInput = armoredInput.GetString(decodedInput);

    if (haveDecodedInput) {
        Letter contents(decodedInput);

        OTASCIIArmor ciphertext = contents.Ciphertext();

        if (ciphertext.Exists()) {
            String nonce = contents.Nonce();

            if (nonce.Exists()) {
                bool haveDecodedCiphertext = ciphertext.GetData(decodedCiphertext);

                if (haveDecodedCiphertext) {

                    String macType(contents.MACType());
                    String ephemeralPubkey(contents.EphemeralKey());

                    const OTAsymmetricKey& privateKey = theRecipient.GetPrivateEncrKey();
                    OTAsymmetricKey* publicKey = OTAsymmetricKey::KeyFactory(OTAsymmetricKey::SECP256K1);
                    publicKey->SetPublicKey(ephemeralPubkey);

                    std::unique_ptr<OTPassword> ECDHSecret(CryptoEngine::Instance().AES().InstantiateBinarySecret());
                    ECDH(*publicKey, privateKey, *ECDHSecret);

                    OTPassword sharedSecret;
                    CryptoEngine::Instance().Hash().HMAC(ECDHDefaultHMAC, *ECDHSecret, nonce, sharedSecret);

                    OTPassword sessionKey(sharedSecret);

                    CryptoEngine::Instance().Hash().Digest(CryptoHash::StringToHashType(macType), nonce, nonceHash);

                    OTPassword truncatedSessionKey(sessionKey.getMemory(), CryptoConfig::SymmetricKeySize());
                    OTData truncatedNonceHash(nonceHash.GetPointer(), CryptoConfig::SymmetricIvSize());

                    bool decrypted = CryptoEngine::Instance().AES().Decrypt(
                        truncatedSessionKey,
                        static_cast<const char*>(decodedCiphertext.GetPointer()),
                        decodedCiphertext.GetSize(),
                        truncatedNonceHash,
                        plaintext
                    );

                    if (decrypted) {
                        theOutput.Set(static_cast<const char*>(plaintext.GetPointer()), plaintext.GetSize());
                        return true;
                    } else {
                        otErr << "Libsecp256k1::" << __FUNCTION__ << " Decryption failed.\n";
                        return false;
                    }
                } else {
                    otErr << "Libsecp256k1::" << __FUNCTION__ << " Could not decode armored ciphertext.\n";
                    return false;
                }
            } else {
                otErr << "Libsecp256k1::" << __FUNCTION__ << " Could not retrieving the nonce from the Letter.\n";
                return false;
            }
        } else {
            otErr << "Libsecp256k1::" << __FUNCTION__ << " Could not retrieve the encoded ciphertext from the Letter.\n";
            return false;
        }
    } else {
        otErr << "Libsecp256k1::" << __FUNCTION__ << " Could not decode armored input data.\n";
        return false;
    }
}

bool Libsecp256k1::SignContract(
    const String& strContractUnsigned,
    const OTAsymmetricKey& theKey,
    OTSignature& theSignature, // output
    CryptoHash::HashType hashType,
    __attribute__((unused)) const OTPasswordData* pPWData)
{
    OTData hash;
    bool haveDigest = CryptoEngine::Instance().Hash().Digest(hashType, strContractUnsigned, hash);

    if (haveDigest) {
        OTPassword privKey;
        bool havePrivateKey = AsymmetricKeyToECDSAPrivkey(theKey, privKey);

        if (havePrivateKey) {
            secp256k1_ecdsa_signature_t ecdsaSignature;

            bool signatureCreated = secp256k1_ecdsa_sign(
                context_,
                &ecdsaSignature,
                reinterpret_cast<const unsigned char*>(hash.GetPointer()),
                reinterpret_cast<const unsigned char*>(privKey.getMemory()),
                nullptr,
                nullptr);

            if (signatureCreated) {
                bool signatureSet = ECDSASignatureToOTSignature(ecdsaSignature, theSignature);
                return signatureSet;
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

bool Libsecp256k1::VerifySignature(
    const String& strContractToVerify,
    const OTAsymmetricKey& theKey,
    const OTSignature& theSignature,
    const CryptoHash::HashType hashType,
    __attribute__((unused)) const OTPasswordData* pPWData
    ) const
{
    OTData hash;
    bool haveDigest = CryptoEngine::Instance().Hash().Digest(hashType, strContractToVerify, hash);

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
                    reinterpret_cast<const unsigned char*>(hash.GetPointer()),
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

bool Libsecp256k1::ECDH(
    const OTAsymmetricKey& publicKey,
    const OTAsymmetricKey& privateKey,
            OTPassword& secret) const
{
    OTPassword scalar;
    secp256k1_pubkey_t point;

    bool havePrivateKey = AsymmetricKeyToECDSAPrivkey(privateKey, scalar);

    if (havePrivateKey) {
        bool havePublicKey = AsymmetricKeyToECDSAPubkey(publicKey, point);

        if (havePublicKey) {
            secret.SetSize(PrivateKeySize);

            return secp256k1_ecdh(
                context_,
                reinterpret_cast<unsigned char*>(secret.getMemoryWritable()),
                &point,
                static_cast<const unsigned char*>(scalar.getMemory()));
        } else {
            otErr << "Libsecp256k1::" << __FUNCTION__ << " could not obtain public key.\n.";
            return false;
        }
    } else {
        otErr << "Libsecp256k1::" << __FUNCTION__ << " could not obtain private key.\n.";
        return false;
    }
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

String Libsecp256k1::Nonce(uint32_t size) const
{
    OTPassword source;
    source.randomizeMemory(size);

    const uint8_t* nonceStart = static_cast<const uint8_t*>(source.getMemory());
    const uint8_t* nonceEnd = nonceStart + source.getMemorySize();

    String nonce(EncodeBase58Check(nonceStart, nonceEnd));

    return nonce;
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
