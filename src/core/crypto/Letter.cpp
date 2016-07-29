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

#include "opentxs/core/crypto/Letter.hpp"

#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/app/App.hpp"
#include "opentxs/core/crypto/AsymmetricKeyEC.hpp"
#include "opentxs/core/crypto/AsymmetricKeyEd25519.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/core/crypto/AsymmetricKeySecp256k1.hpp"
#endif
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHash.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/CryptoUtil.hpp"
#include "opentxs/core/crypto/Ecdsa.hpp"
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/core/crypto/Libsecp256k1.hpp"
#endif
#include "opentxs/core/crypto/Libsodium.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OpenSSL.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTKeypair.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Tag.hpp"

#include <irrxml/irrXML.hpp>
#include <stdint.h>
#include <ostream>
#include <string>

namespace opentxs
{

void Letter::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    Tag rootNode("letter");

    rootNode.add_attribute("iv", iv_.Get());
    rootNode.add_attribute("tag", tag_.Get());
    rootNode.add_attribute("mode", plaintextMode_.Get());

    for (const auto& key : ephemeralKeys_) {
        TagPtr ephemeralKeyNode =
            std::make_shared<Tag>("dhkey");
        const auto type = OTAsymmetricKey::KeyTypeToString(key.first);
        ephemeralKeyNode->add_attribute("type", type.Get());
        ephemeralKeyNode->add_attribute("value", key.second);
        rootNode.add_tag(ephemeralKeyNode);
    }

    for (auto& it : sessionKeys_) {
        TagPtr sessionKeyNode = std::make_shared<Tag>("sessionkey");
        OTASCIIArmor sessionKey;

        std::get<4>(it)->GetAsciiArmoredData(sessionKey);

        sessionKeyNode->add_attribute("algo", std::get<0>(it).Get());
        sessionKeyNode->add_attribute("hmac", std::get<1>(it).Get());
        sessionKeyNode->add_attribute("nonce", std::get<2>(it).Get());
        sessionKeyNode->add_attribute("tag", std::get<3>(it).Get());
        sessionKeyNode->set_text(sessionKey.Get());

        rootNode.add_tag(sessionKeyNode);
    }

    rootNode.add_tag("ciphertext", ciphertext_.Get());

    std::string str_result;
    rootNode.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());
}

int32_t Letter::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    int32_t nReturnVal = 0;

    const String strNodeName(xml->getNodeName());

    if (strNodeName.Compare("letter")) {
        iv_ = xml->getAttributeValue("iv");
        tag_ = xml->getAttributeValue("tag");
        plaintextMode_ = xml->getAttributeValue("mode");
        nReturnVal = 1;
    } else if (strNodeName.Compare("ciphertext")) {
        if (false ==
            Contract::LoadEncodedTextField(xml, ciphertext_)) {
            otErr << "Error in Letter::ProcessXMLNode: no ciphertext."
                  << std::endl;

            return (-1); // error condition
        }
        nReturnVal = 1;
    } else if (strNodeName.Compare("dhkey")) {
        const auto type =
            OTAsymmetricKey::StringToKeyType(xml->getAttributeValue("type"));
        auto& key = EphemeralKey(type);
        key = xml->getAttributeValue("value");
        nReturnVal = 1;
    } else if (strNodeName.Compare("sessionkey")) {
        String algo, hmac, nonce, tag;
        OTASCIIArmor armoredText;

        algo = xml->getAttributeValue("algo");
        hmac = xml->getAttributeValue("hmac");
        nonce = xml->getAttributeValue("nonce");
        tag = xml->getAttributeValue("tag");

        if (false == Contract::LoadEncodedTextField(xml, armoredText)) {
            otErr << "Error in Letter::ProcessXMLNode: no ciphertext."
                  << std::endl;

            return (-1); // error condition
        } else {
            sessionKeys_.push_back(
                symmetricEnvelope(
                    algo,
                    hmac,
                    nonce,
                    tag,
                    std::make_shared<OTEnvelope>(armoredText)));

            nReturnVal = 1;
        }
    }

    return nReturnVal;
}

Letter::Letter(
    const listOfEphemeralKeys& ephemeralKeys,
    const String& iv,
    const String& tag,
    const String& mode,
    const OTASCIIArmor& ciphertext,
    const listOfSessionKeys& sessionKeys)
        : Contract()
        , ephemeralKeys_(ephemeralKeys)
        , iv_(iv)
        , tag_(tag)
        , plaintextMode_(mode)
        , ciphertext_(ciphertext)
        , sessionKeys_(sessionKeys)

{
    m_strContractType.Set("LETTER");
}

Letter::Letter(
        const String& input)
    : Contract()
{
    m_strContractType.Set("LETTER");
    m_xmlUnsigned = input;
    LoadContractXML();
}

Letter::~Letter()
{
    Release_Letter();
}

void Letter::Release_Letter()
{
}

void Letter::Release()
{
    Release_Letter();

    ot_super::Release();

    m_strContractType.Set("LETTER");
}

bool Letter::Seal(
    const mapOfAsymmetricKeys& RecipPubKeys,
    const String& theInput,
    OTData& dataOutput)
{
    bool haveRecipientsECDSA = false;
    bool haveRecipientsED25519 = false;
    bool haveRecipientsRSA = false;
    mapOfECKeys secp256k1Recipients;
    mapOfECKeys ed25519Recipients;
    mapOfAsymmetricKeys RSARecipients;
    listOfEphemeralKeys dhKeys;

    for (auto& it : RecipPubKeys) {
        switch (it.second->keyType()) {
            case proto::AKEYTYPE_SECP256K1 :
                haveRecipientsECDSA = true;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
                secp256k1Recipients.insert(
                    std::pair<std::string, const AsymmetricKeyEC*>(
                        it.first,
                        static_cast<const AsymmetricKeySecp256k1*>(it.second)));
#endif
                break;
            case proto::AKEYTYPE_ED25519 :
                haveRecipientsED25519 = true;
                ed25519Recipients.insert(
                    std::pair<std::string, const AsymmetricKeyEC*>(
                        it.first,
                        static_cast<const AsymmetricKeyEd25519*>(it.second)));
                break;
            case proto::AKEYTYPE_LEGACY :
                haveRecipientsRSA = true;
                RSARecipients.insert(
                    std::pair<std::string, OTAsymmetricKey*>(
                        it.first,
                        it.second));
                break;
            default :
                otErr << __FUNCTION__
                      << ": Unknown recipient type." << std::endl;
                return false;
        }
    }

    // The plaintext will be encrypted to this symmetric key.
    // The session key will be individually encrypted to every recipient.
    BinarySecret masterSessionKey =
        App::Me().Crypto().AES().InstantiateBinarySecretSP();
    masterSessionKey->randomizeMemory(
        CryptoSymmetric::KeySize(defaultPlaintextMode_));

    // Obtain an iv in both binary form, and base58check String form.
    OTData iv;
    String ivReadable = App::Me().Crypto().Util().Nonce(
        CryptoSymmetric::IVSize(defaultPlaintextMode_), iv);
    OTData tag;

    // Now that we have a session key, encrypt the plaintext
    OTData ciphertext;
    bool encrypted = App::Me().Crypto().AES().Encrypt(
        defaultPlaintextMode_,
        *masterSessionKey,
        iv,
        theInput.Get(),
        theInput.GetLength(),
        ciphertext,
        tag);

    if (encrypted) {
        OTASCIIArmor encodedCiphertext(ciphertext);
        listOfSessionKeys sessionKeys;
        String macType = "null";

        String tagReadable = CryptoUtil::Base58CheckEncode(tag).c_str();

        if (haveRecipientsECDSA) {
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_USING_LIBSECP256K1
            Ecdsa& engine =
                static_cast<Libsecp256k1&>(App::Me().Crypto().SECP256K1());
#endif
            macType =
                CryptoHash::HashTypeToString(CryptoEngine::StandardHash);

            OTPassword dhPrivateKey;
            OTData dhPublicKey;

            if (!engine.RandomKeypair(dhPrivateKey, dhPublicKey)) {
                otErr << __FUNCTION__ << ": Failed to generate ephemeral "
                      << "secp256k1 keypair." << std::endl;

                return false;
            }

            dhKeys[proto::AKEYTYPE_SECP256K1] =
                App::Me().Crypto().Util().Base58CheckEncode(dhPublicKey);

            // Individually encrypt the session key to each recipient and add
            // the encrypted key to the global list of session keys for this
            // letter.
            for (auto it : secp256k1Recipients) {
                symmetricEnvelope encryptedSessionKey(
                    CryptoSymmetric::ModeToString(defaultSessionKeyMode_),
                    CryptoHash::HashTypeToString(CryptoEngine::StandardHash),
                    "",
                    "",
                    nullptr);

                OTPasswordData passwordData("ephemeral");
                std::unique_ptr<OTData> dhPublicKey(new OTData);

                OT_ASSERT(dhPublicKey);

                const auto havePubkey = it.second->GetKey(*dhPublicKey);

                if (!havePubkey) {
                    otErr << __FUNCTION__ << ": Failed to get recipient public "
                          << "key." << std::endl;

                    return false;
                }

                bool haveSessionKey = engine.EncryptSessionKeyECDH(
                                        *masterSessionKey,
                                        dhPrivateKey,
                                        *dhPublicKey,
                                        encryptedSessionKey);
                if (haveSessionKey) {
                    sessionKeys.push_back(encryptedSessionKey);

                } else {
                    otErr << __FUNCTION__ << ": Session key "
                          << "encryption failed." << std::endl;
                    return false;
                }
            }
#else
            otErr << __FUNCTION__ << ": Attempting to Seal to "
                  << "secp256k1 recipients without Libsecp256k1 support."
                  << std::endl;
            return false;
#endif
        }

        if (haveRecipientsED25519) {
            Ecdsa& engine =
                static_cast<Libsodium&>(App::Me().Crypto().ED25519());
            macType =
                CryptoHash::HashTypeToString(CryptoEngine::StandardHash);

            OTPassword dhPrivateKey;
            OTData dhPublicKey;

            if (!engine.RandomKeypair(dhPrivateKey, dhPublicKey)) {
                otErr << __FUNCTION__ << ": Failed to generate ephemeral "
                      << "ed25519 keypair." << std::endl;

                return false;
            }

            dhKeys[proto::AKEYTYPE_ED25519] =
                App::Me().Crypto().Util().Base58CheckEncode(dhPublicKey);

            // Individually encrypt the session key to each recipient and add
            // the encrypted key to the global list of session keys for this
            // letter.
            for (auto it : ed25519Recipients) {
                symmetricEnvelope encryptedSessionKey(
                    CryptoSymmetric::ModeToString(defaultSessionKeyMode_),
                    CryptoHash::HashTypeToString(CryptoEngine::StandardHash),
                    "",
                    "",
                    nullptr);

                OTPasswordData passwordData("ephemeral");
                std::unique_ptr<OTData> dhPublicKey(new OTData);

                OT_ASSERT(dhPublicKey);

                const auto havePubkey = it.second->GetKey(*dhPublicKey);

                if (!havePubkey) {
                    otErr << __FUNCTION__ << ": Failed to get recipient public "
                          << "key." << std::endl;

                    return false;
                }

                bool haveSessionKey = engine.EncryptSessionKeyECDH(
                                        *masterSessionKey,
                                        dhPrivateKey,
                                        *dhPublicKey,
                                        encryptedSessionKey);
                if (haveSessionKey) {
                    sessionKeys.push_back(encryptedSessionKey);

                } else {
                    otErr << __FUNCTION__ << ": Session key "
                          << "encryption failed." << std::endl;
                    return false;
                }
            }
        }

        if (haveRecipientsRSA) {
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_USING_OPENSSL
            OpenSSL& engine = static_cast<OpenSSL&>(App::Me().Crypto().RSA());
#endif

            // Encrypt the session key to all RSA recipients and add the
            // encrypted key to the global list of session keys for this letter.
            OTData ciphertext;

            const bool haveSessionKey = engine.EncryptSessionKey(
                RSARecipients, *masterSessionKey, ciphertext);

            if (haveSessionKey) {
                symmetricEnvelope sessionKeyItem(
                    "",
                    "",
                    "",
                    "",
                    std::make_shared<OTEnvelope>(ciphertext));
                sessionKeys.push_back(sessionKeyItem);

            } else {
                otErr << __FUNCTION__ << ": Session key encryption failed."
                      << std::endl;

                return false;
            }
#else
            otErr << __FUNCTION__ << ": Attempting to Seal to OpenSSL "
                  << "recipients without RSA support." << std::endl;

            return false;
#endif
        }

        // Construct the Letter
        Letter theLetter(
            dhKeys,
            ivReadable,
            tagReadable,
            CryptoSymmetric::ModeToString(defaultPlaintextMode_),
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
            otErr << __FUNCTION__ << ": Encryption failed." << std::endl;
            return false;
    }
    return false;
}

bool Letter::Open(
    const OTData& dataInput,
    const Nym& theRecipient,
    String& theOutput,
    const OTPasswordData* pPWData)
{
    OTASCIIArmor armoredInput(dataInput);
    String decodedInput;
    OTData decodedCiphertext;
    OTData passwordHash;
    BinarySecret sessionKey =
        App::Me().Crypto().AES().InstantiateBinarySecretSP();
    OTData plaintext;

    bool haveDecodedInput = armoredInput.GetString(decodedInput);

    if (!haveDecodedInput) {
        otErr << __FUNCTION__ << " Could not decode armored input data."
              << std::endl;

        return false;
    }

    Letter contents(decodedInput);
    OTASCIIArmor ciphertext = contents.Ciphertext();

    if (!ciphertext.Exists()) {
            otErr << __FUNCTION__ << " Could not retrieve the encoded "
                  << "ciphertext from the Letter." << std::endl;

            return false;
    }

    // Extract and decode the nonce
    OTData iv;
    bool ivDecoded = CryptoUtil::Base58CheckDecode(contents.IV().Get(), iv);

    if (!ivDecoded) {
            otErr << __FUNCTION__ << " Could not retrieve the iv from the "
                  << "Letter." << std::endl;

            return false;
    }

    // Extract and decode the AEAD tag
    OTData tag;
    bool tagDecoded =
        CryptoUtil::Base58CheckDecode(contents.AEADTag().Get(), tag);

    if (!tagDecoded) {
        otErr << __FUNCTION__ << " Could not retrieve the tag from the Letter."
              << std::endl;

        return false;
    }

    // Extract and decode the plaintext encryption mode
    CryptoSymmetric::Mode mode = contents.Mode();

    if (CryptoSymmetric::ERROR_MODE == mode) {
        otErr << __FUNCTION__ << " Unsupported or missing plaintext encryption "
              << "mode." << std::endl;

        return false;
    }

    // Extract and decode the ciphertext
    bool haveDecodedCiphertext = ciphertext.GetData(decodedCiphertext);

    if (!haveDecodedCiphertext) {
        otErr << __FUNCTION__ << " Could not decode armored ciphertext."
              << std::endl;

        return false;
    }

    // Attempt to decrypt the session key
    bool haveSessionKey = false;
    const OTAsymmetricKey& privateKey = theRecipient.GetPrivateEncrKey();
    const auto privateKeyType = privateKey.keyType();
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    const bool rsa = privateKey.keyType() == proto::AKEYTYPE_LEGACY;
#endif
    const bool ed25519 = (privateKey.keyType() == proto::AKEYTYPE_ED25519);
    const bool secp256k1 = (privateKey.keyType() == proto::AKEYTYPE_SECP256K1);
    const bool ec = (ed25519 || secp256k1);

    if (ec) {
        // This pointer does not require cleanup
        const AsymmetricKeyEC* ecKey = nullptr;

        if (secp256k1) {
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
            ecKey = static_cast<const AsymmetricKeySecp256k1*>(&privateKey);
#endif
        } else if (ed25519) {
            ecKey = static_cast<const AsymmetricKeyEd25519*>(&privateKey);
        }

        if (nullptr == ecKey) {
            otErr << __FUNCTION__ << ": Unsupported key type." << std::endl;

            return false;
        }

        // Decode ephemeral public key
        String ephemeralPubkey
            (contents.EphemeralKey(privateKeyType));

        if (!ephemeralPubkey.Exists()) {
            otErr << __FUNCTION__ << ": Need an ephemeral public key for ECDH, "
                  << "but the letter does not contain one." << std::endl;

            return false;
        }

        std::unique_ptr<OTData> dhPublicKey(new OTData);

        OT_ASSERT(dhPublicKey);

        const bool decoded =
            App::Me().Crypto().Util().Base58CheckDecode(
                ephemeralPubkey, *dhPublicKey);

        if (!decoded) {
            otErr << __FUNCTION__ << " Failed to decode ephemeral public key: "
                  << std::endl << "'" << ephemeralPubkey << "'" << std::endl;

            return false;
        }

        // Get all the session keys
        listOfSessionKeys sessionKeys(contents.SessionKeys());
        OTPassword dhPrivateKey;
        Ecdsa& engine = ecKey->ECDSA();

        if (nullptr == pPWData) {
            OTPasswordData password(
                "Please enter your password to decrypt this document.");
            engine.AsymmetricKeyToECPrivatekey(
                *ecKey, password, dhPrivateKey);
        } else {
            engine.AsymmetricKeyToECPrivatekey(
                *ecKey, *pPWData, dhPrivateKey);
        }

        // The only way to know which session key (might) belong to us to try
        // them all
        for (auto& it : sessionKeys) {
            haveSessionKey = engine.DecryptSessionKeyECDH(
                it,
                dhPrivateKey,
                *dhPublicKey,
                *sessionKey);

            if (haveSessionKey) {
                break;
            }
        }
    }

#if OT_CRYPTO_SUPPORTED_KEY_RSA
    if (rsa) {
        // Get all the session keys
        listOfSessionKeys sessionKeys(contents.SessionKeys());
#if OT_CRYPTO_USING_OPENSSL
        OpenSSL& engine = static_cast<OpenSSL&>(App::Me().Crypto().RSA());
#endif

        // The only way to know which session key (might) belong to us to try
        // them all
        for (auto& it : sessionKeys) {
            haveSessionKey = engine.DecryptSessionKey(
                std::get<4>(it)->m_dataContents,
                theRecipient,
                *sessionKey,
                pPWData);

            if (haveSessionKey) {
                break;
            }
        }
    }
#endif

    if (haveSessionKey) {
        bool decrypted = App::Me().Crypto().AES().Decrypt(
            mode,
            *sessionKey,
            iv,
            tag,
            static_cast<const char*>(decodedCiphertext.GetPointer()),
            decodedCiphertext.GetSize(),
            plaintext);

        if (decrypted) {
            theOutput.Set(
                static_cast<const char*>(plaintext.GetPointer()),
                plaintext.GetSize());

            return true;
        } else {
            otErr << __FUNCTION__ << " Decryption failed." << std::endl;

            return false;
        }
    } else {
        otErr << __FUNCTION__ << " Could not decrypt any sessions key. "
              << "Was this message intended for us?" << std::endl;

        return false;
    }
}

std::string& Letter::EphemeralKey(const proto::AsymmetricKeyType& type)
{
    return ephemeralKeys_[type];
}

const String& Letter::IV() const
{
    return iv_;
}

const String& Letter::AEADTag() const
{
    return tag_;
}

CryptoSymmetric::Mode Letter::Mode() const
{
    return CryptoSymmetric::StringToMode(plaintextMode_);
}

const listOfSessionKeys& Letter::SessionKeys() const
{
    return sessionKeys_;
}

const OTASCIIArmor& Letter::Ciphertext() const
{
    return ciphertext_;
}

} // namespace opentxs
