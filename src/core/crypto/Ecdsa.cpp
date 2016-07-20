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
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHash.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTData.hpp"

namespace opentxs
{
bool Ecdsa::AsymmetricKeyToECPrivatekey(
    const OTAsymmetricKey& asymmetricKey,
    const OTPasswordData& passwordData,
    OTPassword& privkey,
    const OTPassword* exportPassword) const
{
    OTData dataPrivkey;
    const bool havePrivateKey = asymmetricKey.GetKey(dataPrivkey);

    if (1 > dataPrivkey.GetSize()) {
        return false;
    }

    if (havePrivateKey) {
        return AsymmetricKeyToECPrivkey(
            dataPrivkey, passwordData, privkey, exportPassword);
    } else {
        return false;
    }
}

bool Ecdsa::AsymmetricKeyToECPrivkey(
    const OTData& asymmetricKey,
    const OTPasswordData& passwordData,
    OTPassword& privkey,
    const OTPassword* exportPassword) const
{
    BinarySecret masterPassword(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());

    if (nullptr == exportPassword) {
        masterPassword = CryptoSymmetric::GetMasterKey(passwordData);
        return ImportECPrivatekey(asymmetricKey, *masterPassword, privkey);
    } else {
        return ImportECPrivatekey(asymmetricKey, *exportPassword, privkey);
    }
}

bool Ecdsa::AsymmetricKeyToECPubkey(
    const OTAsymmetricKey& asymmetricKey,
    OTData& pubkey) const
{
    return asymmetricKey.GetKey(pubkey);
}

bool Ecdsa::DecryptPrivateKey(
    const OTData& encryptedKey,
    const OTPassword& password,
    OTPassword& plaintextKey)
{
    OTPassword keyPassword;
    App::Me().Crypto().Hash().Digest(
        proto::HASHTYPE_SHA256, password, keyPassword);

    return App::Me().Crypto().AES().Decrypt(
        CryptoSymmetric::AES_256_ECB,
        keyPassword,
        static_cast<const char*>(encryptedKey.GetPointer()),
        encryptedKey.GetSize(),
        plaintextKey);
}

bool Ecdsa::DecryptSessionKeyECDH(
    const symmetricEnvelope& encryptedSessionKey,
    const OTAsymmetricKey& privateKey,
    const OTAsymmetricKey& publicKey,
    const OTPasswordData& passwordData,
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
    bool haveECDH = ECDH(publicKey, privateKey, passwordData, *ECDHSecret);

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
    OTAsymmetricKey& asymmetricKey) const
{
    BinarySecret masterPassword(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());

    masterPassword = CryptoSymmetric::GetMasterKey(passwordData, true);

    return ExportECPrivatekey(privkey, *masterPassword, asymmetricKey);
}

bool Ecdsa::ECPubkeyToAsymmetricKey(
    std::unique_ptr<OTData>& pubkey,
    OTAsymmetricKey& asymmetricKey) const
{
    if (!pubkey) { return false; }

    return asymmetricKey.SetKey(pubkey, false);
}

bool Ecdsa::EncryptPrivateKey(
    const OTPassword& plaintextKey,
    const OTPassword& password,
    OTData& encryptedKey)
{
    OTPassword keyPassword;
    App::Me().Crypto().Hash().Digest(
        proto::HASHTYPE_SHA256, password, keyPassword);

    return App::Me().Crypto().AES().Encrypt(
        CryptoSymmetric::AES_256_ECB,
        keyPassword,
        static_cast<const char*>(plaintextKey.getMemory()),
        plaintextKey.getMemorySize(),
        encryptedKey);
}

bool Ecdsa::EncryptSessionKeyECDH(
    const OTPassword& sessionKey,
    const OTAsymmetricKey& privateKey,
    const OTAsymmetricKey& publicKey,
    const OTPasswordData& passwordData,
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

    OTData nonce;
    String nonceReadable =
        App::Me().Crypto().Util().Nonce(CryptoSymmetric::KeySize(algo), nonce);

    // Calculate ECDH shared secret
    BinarySecret ECDHSecret(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());
    const bool haveECDH =
        ECDH(publicKey, privateKey, passwordData, *ECDHSecret);

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
    const OTPassword& password,
    OTAsymmetricKey& asymmetricKey) const
{
    std::unique_ptr<OTData> encryptedKey(new OTData());

    OT_ASSERT(encryptedKey);

    EncryptPrivateKey(privkey, password, *encryptedKey);

    return asymmetricKey.SetKey(encryptedKey, true);
}

bool Ecdsa::ImportECPrivatekey(
    const OTData& asymmetricKey,
    const OTPassword& password,
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
    OTData encryptedKey(
        privateKey.key().c_str(),
        privateKey.key().size());
    BinarySecret plaintextKey(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());
    BinarySecret masterPassword(
        App::Me().Crypto().AES().InstantiateBinarySecretSP());
    masterPassword = CryptoSymmetric::GetMasterKey("");
    const bool decrypted = Ecdsa::DecryptPrivateKey(
        encryptedKey,
        *masterPassword,
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
} // namespace opentxs
