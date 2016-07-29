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

#include "opentxs/core/crypto/Ecdsa.hpp"

#include "opentxs/core/app/App.hpp"
#include "opentxs/core/crypto/AsymmetricKeyEC.hpp"
#include "opentxs/core/crypto/Crypto.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHash.hpp"
#include "opentxs/core/crypto/CryptoHashEngine.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/CryptoSymmetricEngine.hpp"
#include "opentxs/core/crypto/CryptoUtil.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTData.hpp"

namespace opentxs
{
bool Ecdsa::AsymmetricKeyToECPrivatekey(
    const AsymmetricKeyEC& asymmetricKey,
    const OTPasswordData& passwordData,
    OTPassword& privkey) const
{
    proto::Ciphertext dataPrivkey;
    const bool havePrivateKey = asymmetricKey.GetKey(dataPrivkey);

    if (!havePrivateKey) { return false; }

    return AsymmetricKeyToECPrivkey(
        dataPrivkey, passwordData, privkey);
}

bool Ecdsa::AsymmetricKeyToECPrivkey(
    const proto::Ciphertext& asymmetricKey,
    const OTPasswordData& passwordData,
    OTPassword& privkey) const
{
    BinarySecret masterPassword(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());

    return ImportECPrivatekey(asymmetricKey, passwordData, privkey);
}

bool Ecdsa::AsymmetricKeyToECPubkey(
    const AsymmetricKeyEC& asymmetricKey,
    OTData& pubkey) const
{
    return asymmetricKey.GetKey(pubkey);
}

bool Ecdsa::DecryptPrivateKey(
    const proto::Ciphertext& encryptedKey,
    const OTPasswordData& password,
    OTPassword& plaintextKey)
{
    auto key = App::Me().Crypto().Symmetric().Key(
        encryptedKey.key(),
        encryptedKey.mode());

    return key->Decrypt(encryptedKey, password, plaintextKey);
}

bool Ecdsa::DecryptPrivateKey(
    const proto::Ciphertext& encryptedKey,
    const proto::Ciphertext& encryptedChaincode,
    const OTPasswordData& password,
    OTPassword& key,
    OTPassword& chaincode)
{
    auto sessionKey = App::Me().Crypto().Symmetric().Key(
        encryptedKey.key(),
        encryptedKey.mode());

    const bool keyDecrypted =
        sessionKey->Decrypt(encryptedKey, password, key);
    const bool chaincodeDecrypted =
        sessionKey->Decrypt(encryptedChaincode, password, chaincode);

    return (keyDecrypted && chaincodeDecrypted);
}

bool Ecdsa::DecryptSessionKeyECDH(
    const symmetricEnvelope& encryptedSessionKey,
    const OTPassword& privateKey,
    const OTData& publicKey,
    OTPassword& sessionKey) const
{
    const CryptoSymmetric::Mode algo =
        CryptoSymmetric::StringToMode(std::get<0>(encryptedSessionKey));
    const proto::HashType hmac =
        CryptoHash::StringToHashType(std::get<1>(encryptedSessionKey));

    if (CryptoSymmetric::ERROR_MODE == algo) {
        otErr << __FUNCTION__ << ": Unsupported encryption algorithm."
              << std::endl;

        return false;
    }

    if (proto::HASHTYPE_ERROR == hmac) {
        otErr << __FUNCTION__ << ": Unsupported hmac algorithm."
              << std::endl;

        return false;
    }

    // Extract and decode the nonce
    OTData nonce;
    bool nonceDecoded =
        CryptoUtil::Base58CheckDecode(
            std::get<2>(encryptedSessionKey).Get(), nonce);

    if (!nonceDecoded) {
        otErr << __FUNCTION__ << ": Can not decode nonce." << std::endl;

        return false;
    }

    // Calculate ECDH shared secret
    BinarySecret ECDHSecret(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());
    bool haveECDH = ECDH(publicKey, privateKey, *ECDHSecret);

    if (!haveECDH) {
        otErr << __FUNCTION__ << ": ECDH shared secret negotiation failed."
              << std::endl;

        return false;
    }

    // In order to make sure the session key is always encrypted to a different
    // key for every Seal() action even if the sender and recipient are the
    // same, don't use the ECDH secret directly. Instead, calculate an HMAC of
    // the shared secret and a nonce and use that as the AES encryption key.
    BinarySecret sharedSecret(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());
    App::Me().Crypto().Hash().HMAC(hmac, *ECDHSecret, nonce, *sharedSecret);

    // The values calculated above might not be the correct size for the default
    // symmetric encryption function.

    const bool goodSize =
        ((sharedSecret->getMemorySize() >= CryptoConfig::SymmetricKeySize()) &&
        (nonce.GetSize() >= CryptoConfig::SymmetricIvSize()));

    if (!goodSize) {
        otErr << __FUNCTION__ << ": Insufficient nonce or key size."
              << std::endl;

        return false;
    }

    BinarySecret truncatedSharedSecret(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());
    truncatedSharedSecret->setMemory(
        sharedSecret->getMemory(), CryptoSymmetric::KeySize(algo));
    OTData truncatedNonce(nonce.GetPointer(), CryptoSymmetric::IVSize(algo));

    // Extract and decode the tag from the envelope
    OTData tag;
    CryptoUtil::Base58CheckDecode(std::get<3>(encryptedSessionKey).Get(), tag);

    // Extract and decode the ciphertext from the envelope
    OTData ciphertext;
    OTASCIIArmor encodedCiphertext;
    std::get<4>(encryptedSessionKey)->GetAsciiArmoredData(encodedCiphertext);
    encodedCiphertext.GetData(ciphertext);

    return App::Me().Crypto().AES().Decrypt(
        algo,
        *truncatedSharedSecret,
        truncatedNonce,
        tag,
        static_cast<const char*>(ciphertext.GetPointer()),
        ciphertext.GetSize(),
        sessionKey);
}

bool Ecdsa::ECPrivatekeyToAsymmetricKey(
    const OTPassword& privkey,
    const OTPasswordData& passwordData,
    AsymmetricKeyEC& asymmetricKey) const
{
    return ExportECPrivatekey(privkey, passwordData, asymmetricKey);
}

bool Ecdsa::ECPubkeyToAsymmetricKey(
    std::unique_ptr<OTData>& pubkey,
    AsymmetricKeyEC& asymmetricKey) const
{
    if (!pubkey) { return false; }

    return asymmetricKey.SetKey(pubkey);
}

bool Ecdsa::EncryptPrivateKey(
    const OTPassword& plaintextKey,
    const OTPasswordData& password,
    proto::Ciphertext& encryptedKey)
{
    auto key = App::Me().Crypto().Symmetric().Key(password);

    if (!key) { return false; }

    OTData blank;

    return key->Encrypt(
        plaintextKey,
        blank,
        password,
        encryptedKey,
        true);
}

bool Ecdsa::EncryptPrivateKey(
    const OTPassword& key,
    const OTPassword& chaincode,
    const OTPasswordData& password,
    proto::Ciphertext& encryptedKey,
    proto::Ciphertext& encryptedChaincode)
{
    auto sessionKey = App::Me().Crypto().Symmetric().Key(password);

    if (!sessionKey) { return false; }

    OTData blank;

    const bool keyEncrypted = sessionKey->Encrypt(
        key,
        blank,
        password,
        encryptedKey,
        true);

    const bool chaincodeEncrypted = sessionKey->Encrypt(
        chaincode,
        blank,
        password,
        encryptedChaincode,
        false);

    return (keyEncrypted && chaincodeEncrypted);
}

bool Ecdsa::EncryptSessionKeyECDH(
    const OTPassword& sessionKey,
    const OTPassword& privateKey,
    const OTData& publicKey,
    symmetricEnvelope& encryptedSessionKey) const
{
    CryptoSymmetric::Mode algo =
        CryptoSymmetric::StringToMode(std::get<0>(encryptedSessionKey));
    proto::HashType hmac =
        CryptoHash::StringToHashType(std::get<1>(encryptedSessionKey));

    if (CryptoSymmetric::ERROR_MODE == algo) {
        otErr << __FUNCTION__ << ": Unsupported encryption algorithm."
              << std::endl;

        return false;
    }

    if (proto::HASHTYPE_ERROR == hmac) {
        otErr << __FUNCTION__ << ": Unsupported hmac algorithm."
              << std::endl;

        return false;
    }

    if (!sessionKey.isMemory()) { return false; }

    OTData nonce;
    String nonceReadable =
        App::Me().Crypto().Util().Nonce(CryptoSymmetric::KeySize(algo), nonce);

    // Calculate ECDH shared secret
    BinarySecret ECDHSecret(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());
    const bool haveECDH =
        ECDH(publicKey, privateKey, *ECDHSecret);

    if (!haveECDH) {
        otErr << __FUNCTION__ << ": ECDH shared secret negotiation failed."
              << std::endl;

        return false;
    }

    // In order to make sure the session key is always encrypted to a different
    // key for every Seal() action, even if the sender and recipient are the
    // same, don't use the ECDH secret directly. Instead, calculate an HMAC of
    // the shared secret and a nonce and use that as the AES encryption key.
    BinarySecret sharedSecret(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());
    App::Me().Crypto().Hash().HMAC(hmac, *ECDHSecret, nonce, *sharedSecret);

    // The values calculated above might not be the correct size for the default
    // symmetric encryption function.
    const bool goodSize =
        ((sharedSecret->getMemorySize() >= CryptoSymmetric::KeySize(algo)) &&
        (nonce.GetSize() >= CryptoSymmetric::IVSize(algo)));

    if (!goodSize) {
        otErr << __FUNCTION__ << ": Insufficient nonce or key size."
              << std::endl;

        return false;
    }

    BinarySecret truncatedSharedSecret(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());
    truncatedSharedSecret->setMemory(
        sharedSecret->getMemory(), CryptoSymmetric::KeySize(algo));
    OTData truncatedNonce(nonce.GetPointer(), CryptoSymmetric::IVSize(algo));

    OTData ciphertext, tag;
    bool encrypted = App::Me().Crypto().AES().Encrypt(
        algo,
        *truncatedSharedSecret,
        truncatedNonce,
        static_cast<const char*>(sessionKey.getMemory()),
        sessionKey.getMemorySize(),
        ciphertext,
        tag);

    if (!encrypted) {
        otErr << __FUNCTION__ << ": Session key encryption failed."
              << std::endl;

        return false;
    }

    OTASCIIArmor encodedCiphertext(ciphertext);
    String tagReadable(CryptoUtil::Base58CheckEncode(tag));

    std::get<2>(encryptedSessionKey) = nonceReadable;
    std::get<3>(encryptedSessionKey) = tagReadable;
    std::get<4>(encryptedSessionKey) =
        std::make_shared<OTEnvelope>(encodedCiphertext);

    return true;
}

bool Ecdsa::ExportECPrivatekey(
    const OTPassword& privkey,
    const OTPasswordData& password,
    AsymmetricKeyEC& asymmetricKey) const
{
    std::unique_ptr<proto::Ciphertext> encryptedKey(new proto::Ciphertext);

    EncryptPrivateKey(privkey, password, *encryptedKey);

    return asymmetricKey.SetKey(encryptedKey);
}

bool Ecdsa::ImportECPrivatekey(
    const proto::Ciphertext& asymmetricKey,
    const OTPasswordData& password,
    OTPassword& privkey) const
{
    return DecryptPrivateKey(asymmetricKey, password, privkey);
}

bool Ecdsa::PrivateToPublic(
    const proto::AsymmetricKey& privateKey,
    proto::AsymmetricKey& publicKey) const
{
    publicKey.CopyFrom(privateKey);
    publicKey.clear_chaincode();
    publicKey.clear_key();
    publicKey.set_mode(proto::KEYMODE_PUBLIC);
    BinarySecret plaintextKey(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());
    OTPasswordData password(__FUNCTION__);
    const bool decrypted = DecryptPrivateKey(
        privateKey.encryptedkey(),
        password,
        *plaintextKey);

    if (!decrypted) { return false; }

    OTData key;
    const bool output = ScalarBaseMultiply(*plaintextKey, key);

    if (output) {
        publicKey.set_key(key.GetPointer(), key.GetSize());

        return true;
    }

    return false;
}

bool Ecdsa::SeedToCurveKey(
    __attribute__((unused)) const OTPassword& seed,
    __attribute__((unused)) OTPassword& privateKey,
    __attribute__((unused)) OTData& publicKey) const
{
    otErr << __FUNCTION__ << ": this engine does not support curve25519."
          << std::endl;

    return false;
}
} // namespace opentxs
