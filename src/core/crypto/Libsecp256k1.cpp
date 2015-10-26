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
#include <opentxs/core/crypto/Crypto.hpp>
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
#include <opentxs/core/crypto/Letter.hpp>

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
    // Eventually, this function will be moved out of Libsecp256k1 and into Letter.
    // Until that happens, the lines below are redundant.
    bool haveRecipientsECDSA = false;
    mapOfAsymmetricKeys secp256k1Recipients;

    for (auto it : RecipPubKeys) {
        if (it.second->keyType() == OTAsymmetricKey::SECP256K1) {
            haveRecipientsECDSA = true;
            secp256k1Recipients.insert(std::pair<std::string, OTAsymmetricKey*>(it.first, it.second));
        }
    }

    // The plaintext will be encrypted to this symmetric key.
    // The session key will be individually encrypted to every recipient.
    BinarySecret masterSessionKey = CryptoEngine::Instance().AES().InstantiateBinarySecretSP();
    // Obtain an iv in both binary form, and base58check String form.
    OTData iv;
    String ivReadable = Nonce(CryptoConfig::SymmetricIvSize(), iv);

    // Now that we have a session key, encrypt the plaintext
    OTData ciphertext;
    bool encrypted = CryptoEngine::Instance().AES().Encrypt(
        *masterSessionKey,
        theInput.Get(),
        theInput.GetLength(),
        iv,
        ciphertext
    );

    if (encrypted) {
        OTASCIIArmor encodedCiphertext(ciphertext);
        FormattedKey ephemeralPubkey;
        mapOfSessionKeys sessionKeys;

        if (haveRecipientsECDSA) {
            #if defined(OT_CRYPTO_USING_LIBSECP256K1)

            // Generate an emphemeral keypair for ECDH shared secret derivation.
            // Why not use the sender's secp256k1 key for this?
            // Because maybe the sender only has RSA credentials.
            OTKeypair ephemeralKeypair(OTAsymmetricKey::SECP256K1);
            std::shared_ptr<NymParameters> pKeyData;
            pKeyData = std::make_shared<NymParameters>(
                NymParameters::SECP256K1,
                Credential::SECP256K1_PUBKEY);
            ephemeralKeypair.MakeNewKeypair(pKeyData);
            ephemeralKeypair.GetPublicKey(ephemeralPubkey);

            const OTAsymmetricKey& ephemeralPrivkey = ephemeralKeypair.GetPrivateKey();

            // Indididually encrypt the session key to each recipient and add the encrypted key
            // to the global list of session keys for this letter.
            for (auto it : secp256k1Recipients) {
                std::pair<String, OTEnvelope> encryptedSessionKey;

                bool haveSessionKey = EncryptSessionKeyECDH(
                                        *masterSessionKey,
                                        ephemeralPrivkey,
                                        *(it.second),
                                        encryptedSessionKey);
                if (haveSessionKey) {
                    sessionKeys.insert(encryptedSessionKey);

                } else {
                    otErr << "Libsecp256k1::" << __FUNCTION__ << ": Session key encryption failed.\n";
                    return false;
                }
            }
            #else

            otErr << "Libsecp256k1::" << __FUNCTION__ << ": Attempting to Seal to secp256k1 recipients"
                <<" without libsecp256k1 support.\n";
            return false;
            #endif
        }

        // Construct the Letter
        Letter theLetter(
            ephemeralPubkey,
            CryptoHash::HashTypeToString(ECDHDefaultHMAC),
            ivReadable,
            encodedCiphertext,
            sessionKeys
        );

        // Serialize the Letter to a String
        String output;
        theLetter.UpdateContents();
        theLetter.SaveContents(output);

        //Encode the serialized Letter into OTData and set the output
        OTASCIIArmor armoredOutput(output);
        OTData finishedOutput(armoredOutput);
        dataOutput.Assign(finishedOutput);

        return true;
    } else {
            otErr << "Libsecp256k1::" << __FUNCTION__ << ": Encryption failed.\n";
            return false;
    }
    return false;
}

bool Libsecp256k1::Open(
    OTData& dataInput,
    const Nym& theRecipient,
    String& theOutput,
    __attribute__((unused)) const OTPasswordData* pPWData
    ) const
{
    OTASCIIArmor armoredInput(dataInput);
    String decodedInput;
    OTData decodedCiphertext;
    OTData passwordHash;
    OTPassword sessionKey;
    OTData plaintext;

    bool haveDecodedInput = armoredInput.GetString(decodedInput);

    if (haveDecodedInput) {
        Letter contents(decodedInput);

        OTASCIIArmor ciphertext = contents.Ciphertext();

        if (ciphertext.Exists()) {
            // Extract and decode the nonce
            std::vector<unsigned char> decodedIV;
            bool ivDecoded = DecodeBase58Check(contents.IV().Get(), decodedIV);

            if (ivDecoded) {
                OTData iv(decodedIV);
                bool haveDecodedCiphertext = ciphertext.GetData(decodedCiphertext);

                if (haveDecodedCiphertext) {
                    // Decode hmac type
                    CryptoHash::HashType macType = CryptoHash::StringToHashType(contents.MACType());

                    bool haveSessionKey = false;
                    const OTAsymmetricKey& privateKey = theRecipient.GetPrivateEncrKey();

                    // This if statement exists so that the function can eventually support
                    // RSA recipients as well.
                    if (privateKey.keyType() == OTAsymmetricKey::SECP256K1) {
                        // Decode ephemeral public key
                        String ephemeralPubkey(contents.EphemeralKey());

                        if(ephemeralPubkey.Exists()) {
                            OTAsymmetricKey* publicKey = OTAsymmetricKey::KeyFactory(OTAsymmetricKey::SECP256K1);
                            publicKey->SetPublicKey(ephemeralPubkey);

                            // Get all the session keys
                            mapOfSessionKeys sessionKeys(contents.SessionKeys());

                            // The only way to know which session key (might) belong to us to try them all
                            OTPassword sessionKey;

                            for (auto it : sessionKeys) {
                                Libsecp256k1& engine = static_cast<Libsecp256k1&>(privateKey.engine());

                                haveSessionKey = engine.DecryptSessionKeyECDH(
                                                        it,
                                                        macType,
                                                        privateKey,
                                                        *publicKey,
                                                        sessionKey
                                                    );
                                if (haveSessionKey) {
                                    break;
                                }
                            }
                            // We're done with this
                            if (nullptr != publicKey) {
                                delete publicKey;
                                publicKey = nullptr;
                            }
                        }
                        if (haveSessionKey) {
                            bool decrypted = CryptoEngine::Instance().AES().Decrypt(
                                sessionKey,
                                static_cast<const char*>(decodedCiphertext.GetPointer()),
                                decodedCiphertext.GetSize(),
                                iv,
                                plaintext);

                            if (decrypted) {
                                theOutput.Set(static_cast<const char*>(plaintext.GetPointer()), plaintext.GetSize());
                                return true;
                            } else {
                                otErr << "Libsecp256k1::" << __FUNCTION__ << " Decryption failed.\n";
                                return false;
                            }
                        } else {
                            otErr << "Libsecp256k1::" << __FUNCTION__ << " Could not decrypt any sessions key. "
                                << "Was this message intended for us?\n";
                            return false;
                        }
                    } else {
                        otErr << "Libsecp256k1::" << __FUNCTION__ << " Need an ephemeral public key for ECDH, but "
                            << "the letter does not contain one.\n";
                        return false;
                    }
                } else {
                    otErr << "Libsecp256k1::" << __FUNCTION__ << " Could not decode armored ciphertext.\n";
                    return false;
                }
            } else {
                otErr << "Libsecp256k1::" << __FUNCTION__ << " Could not retrieve the iv from the Letter.\n";
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

bool Libsecp256k1::EncryptSessionKeyECDH(
        const OTPassword& sessionKey,
        const OTAsymmetricKey& privateKey,
        const OTAsymmetricKey& publicKey,
        std::pair<String, OTEnvelope>& encryptedSessionKey) const
{
    OTData nonce;
    String nonceReadable = Nonce(ECDHDefaultHMACSize, nonce);

    // Calculate ECDH shared secret
    BinarySecret ECDHSecret(CryptoEngine::Instance().AES().InstantiateBinarySecretSP());
    bool haveECDH = ECDH(publicKey, privateKey, *ECDHSecret);

    if (haveECDH) {
        // In order to make sure the session key is always encrypted to a different key for every Seal() action,
        // even if the sender and recipient are the same, don't use the ECDH secret directly. Instead, calculate
        // an HMAC of the shared secret and a nonce and use that as the AES encryption key.
        OTPassword sharedSecret;
        CryptoEngine::Instance().Hash().HMAC(ECDHDefaultHMAC, *ECDHSecret, nonce, sharedSecret);

        // The values calculated above might not be the correct size for the default symmetric encryption
        // function.
        if (
            (sharedSecret.getMemorySize() >= CryptoConfig::SymmetricKeySize()) &&
            (nonce.GetSize() >= CryptoConfig::SymmetricIvSize())) {

                OTPassword truncatedSharedSecret(sharedSecret.getMemory(), CryptoConfig::SymmetricKeySize());
                OTData truncatedNonce(nonce.GetPointer(), CryptoConfig::SymmetricIvSize());

                OTData ciphertext;
                bool encrypted = CryptoEngine::Instance().AES().Encrypt(
                    truncatedSharedSecret,
                    static_cast<const char*>(sessionKey.getMemory()),
                    sessionKey.getMemorySize(),
                    truncatedNonce,
                    ciphertext);

                    if (encrypted) {
                        OTASCIIArmor encodedCiphertext(ciphertext);
                        OTEnvelope sessionKeyEnvelope(encodedCiphertext);

                        encryptedSessionKey.first = nonceReadable;
                        encryptedSessionKey.second = sessionKeyEnvelope;

                        return true;
                    } else {
                        otErr << "Libsecp256k1::" << __FUNCTION__ << ": Session key encryption failed.\n";
                        return false;
                    }
        } else {
            otErr << "Libsecp256k1::" << __FUNCTION__ << ": Insufficient nonce or key size.\n";
            return false;
        }

    } else {
        otErr << "Libsecp256k1::" << __FUNCTION__ << ": ECDH shared secret negotiation failed.\n";
        return false;
    }
}

bool Libsecp256k1::DecryptSessionKeyECDH(
        const std::pair<String, OTEnvelope>& encryptedSessionKey,
        const CryptoHash::HashType macType,
        const OTAsymmetricKey& privateKey,
        const OTAsymmetricKey& publicKey,
        OTPassword& sessionKey) const
{
    // Extract and decode the nonce
    std::vector<unsigned char> decodedNonce;
    bool nonceDecoded = DecodeBase58Check(encryptedSessionKey.first.Get(), decodedNonce);

    if (nonceDecoded) {
        OTData nonce(decodedNonce);

        // Calculate ECDH shared secret
        BinarySecret ECDHSecret(CryptoEngine::Instance().AES().InstantiateBinarySecretSP());
        bool haveECDH = ECDH(publicKey, privateKey, *ECDHSecret);

        if (haveECDH) {
            // In order to make sure the session key is always encrypted to a different key for every Seal() action
            // even if the sender and recipient are the same, don't use the ECDH secret directly. Instead, calculate
            // an HMAC of the shared secret and a nonce and use that as the AES encryption key.
            OTPassword sharedSecret;
            CryptoEngine::Instance().Hash().HMAC(macType, *ECDHSecret, nonce, sharedSecret);

            // The values calculated above might not be the correct size for the default symmetric encryption
            // function.
            if (
                (sharedSecret.getMemorySize() >= CryptoConfig::SymmetricKeySize()) &&
                (nonce.GetSize() >= CryptoConfig::SymmetricIvSize())) {

                    OTPassword truncatedSharedSecret(sharedSecret.getMemory(), CryptoConfig::SymmetricKeySize());
                    OTData truncatedNonce(nonce.GetPointer(), CryptoConfig::SymmetricIvSize());

                    // Extract and decode the ciphertext from the envelope
                    OTData ciphertext;
                    OTASCIIArmor encodedCiphertext;
                    encryptedSessionKey.second.GetAsciiArmoredData(encodedCiphertext);
                    encodedCiphertext.GetData(ciphertext);

                    return CryptoEngine::Instance().AES().Decrypt(
                                                            truncatedSharedSecret,
                                                            static_cast<const char*>(ciphertext.GetPointer()),
                                                            ciphertext.GetSize(),
                                                            truncatedNonce,
                                                            sessionKey);
            } else {
                otErr << "Libsecp256k1::" << __FUNCTION__ << ": Insufficient nonce or key size.\n";
                return false;
            }

        } else {
            otErr << "Libsecp256k1::" << __FUNCTION__ << ": ECDH shared secret negotiation failed.\n";
            return false;
        }
    } else {
        otErr << "Libsecp256k1::" << __FUNCTION__ << ": Can not decode nonce.\n";
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

String Libsecp256k1::Nonce(const uint32_t size) const
{
    OTData unusedOutput;
    return Nonce(size, unusedOutput);
}

String Libsecp256k1::Nonce(const uint32_t size, OTData& rawOutput) const
{
    rawOutput.zeroMemory();
    rawOutput.SetSize(size);

    OTPassword source;
    source.randomizeMemory(size);

    const uint8_t* nonceStart = static_cast<const uint8_t*>(source.getMemory());
    const uint8_t* nonceEnd = nonceStart + source.getMemorySize();

    String nonce(EncodeBase58Check(nonceStart, nonceEnd));

    rawOutput.Assign(source.getMemory(), source.getMemorySize());
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
