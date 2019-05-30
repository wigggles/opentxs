// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"

#include "EcdsaProvider.hpp"

#define OT_METHOD "opentxs::EcdsaProvider::"

namespace opentxs::crypto::implementation
{
EcdsaProvider::EcdsaProvider(const api::Crypto& crypto)
    : crypto_(crypto)
{
}

bool EcdsaProvider::AsymmetricKeyToECPrivatekey(
    const api::Core& api,
    const crypto::key::EllipticCurve& asymmetricKey,
    const PasswordPrompt& reason,
    OTPassword& privkey) const
{
    proto::Ciphertext dataPrivkey;
    const bool havePrivateKey = asymmetricKey.GetKey(dataPrivkey);

    if (!havePrivateKey) { return false; }

    return AsymmetricKeyToECPrivkey(api, dataPrivkey, reason, privkey);
}

bool EcdsaProvider::AsymmetricKeyToECPrivkey(
    const api::Core& api,
    const proto::Ciphertext& asymmetricKey,
    const PasswordPrompt& reason,
    OTPassword& privkey) const
{
    return ImportECPrivatekey(api, asymmetricKey, reason, privkey);
}

bool EcdsaProvider::AsymmetricKeyToECPubkey(
    const crypto::key::EllipticCurve& asymmetricKey,
    Data& pubkey) const
{
    return asymmetricKey.GetKey(pubkey);
}

bool EcdsaProvider::DecryptPrivateKey(
    const api::Core& api,
    const proto::Ciphertext& encryptedKey,
    const PasswordPrompt& reason,
    OTPassword& plaintextKey) const
{
    auto key = api.Symmetric().Key(encryptedKey.key(), encryptedKey.mode());

    if (!key.get()) { return false; }

    return key->Decrypt(encryptedKey, reason, plaintextKey);
}

bool EcdsaProvider::DecryptPrivateKey(
    const api::Core& api,
    const proto::Ciphertext& encryptedKey,
    const proto::Ciphertext& encryptedChaincode,
    const PasswordPrompt& reason,
    OTPassword& key,
    OTPassword& chaincode) const
{
    auto sessionKey =
        api.Symmetric().Key(encryptedKey.key(), encryptedKey.mode());
    const bool keyDecrypted = sessionKey->Decrypt(encryptedKey, reason, key);
    const bool chaincodeDecrypted =
        sessionKey->Decrypt(encryptedChaincode, reason, chaincode);

    return (keyDecrypted && chaincodeDecrypted);
}

bool EcdsaProvider::DecryptSessionKeyECDH(
    const api::Core& api,
    const crypto::key::EllipticCurve& privateKey,
    const crypto::key::EllipticCurve& publicKey,
    crypto::key::Symmetric& sessionKey,
    PasswordPrompt& sessionKeyPassword,
    const PasswordPrompt& reason) const
{
    OTPassword plaintextKey{};

    return DecryptSessionKeyECDH(
        api,
        privateKey,
        publicKey,
        sessionKey,
        sessionKeyPassword,
        plaintextKey,
        reason);
}

bool EcdsaProvider::DecryptSessionKeyECDH(
    const api::Core& api,
    const crypto::key::EllipticCurve& privateKey,
    const crypto::key::EllipticCurve& publicKey,
    crypto::key::Symmetric& sessionKey,
    PasswordPrompt& sessionKeyPassword,
    OTPassword& plaintextKey,
    const PasswordPrompt& reason) const
{
    auto publicDHKey = Data::Factory();

    if (!publicKey.GetKey(publicDHKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get public key.")
            .Flush();

        return false;
    }

    OTPassword privateDHKey;

    if (!AsymmetricKeyToECPrivatekey(api, privateKey, reason, privateDHKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get private key.")
            .Flush();

        return false;
    }

    // Calculate ECDH shared secret
    auto ECDHSecret = api.Factory().BinarySecret();

    OT_ASSERT(ECDHSecret);

    bool haveECDH = ECDH(publicDHKey, privateDHKey, *ECDHSecret);

    if (!haveECDH) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": ECDH shared secret negotiation failed.")
            .Flush();

        return false;
    }

    plaintextKey = *ECDHSecret;
    sessionKeyPassword.SetPassword(plaintextKey);

    return sessionKey.Unlock(sessionKeyPassword);
}

bool EcdsaProvider::ECPrivatekeyToAsymmetricKey(
    const api::Core& api,
    const OTPassword& privkey,
    const PasswordPrompt& reason,
    crypto::key::EllipticCurve& asymmetricKey) const
{
    return ExportECPrivatekey(api, privkey, reason, asymmetricKey);
}

bool EcdsaProvider::ECPubkeyToAsymmetricKey(
    const Data& pubkey,
    crypto::key::EllipticCurve& asymmetricKey) const
{
    if (pubkey.empty()) { return false; }

    return asymmetricKey.SetKey(pubkey);
}

bool EcdsaProvider::EncryptPrivateKey(
    const api::Core& api,
    const OTPassword& plaintextKey,
    const PasswordPrompt& reason,
    proto::Ciphertext& encryptedKey) const
{
    auto key = api.Symmetric().Key(reason);

    if (!key.get()) { return false; }

    auto blank = Data::Factory();

    return key->Encrypt(plaintextKey, blank, reason, encryptedKey, true);
}

bool EcdsaProvider::EncryptPrivateKey(
    const api::Core& api,
    const OTPassword& key,
    const OTPassword& chaincode,
    const PasswordPrompt& reason,
    proto::Ciphertext& encryptedKey,
    proto::Ciphertext& encryptedChaincode) const
{
    auto sessionKey = api.Symmetric().Key(reason);

    if (!sessionKey.get()) { return false; }

    auto blank = Data::Factory();
    const bool keyEncrypted =
        sessionKey->Encrypt(key, blank, reason, encryptedKey, true);
    const bool chaincodeEncrypted = sessionKey->Encrypt(
        chaincode, blank, reason, encryptedChaincode, false);

    return (keyEncrypted && chaincodeEncrypted);
}

bool EcdsaProvider::EncryptSessionKeyECDH(
    const api::Core& api,
    const crypto::key::EllipticCurve& privateKey,
    const crypto::key::EllipticCurve& publicKey,
    crypto::key::Symmetric& sessionKey,
    const PasswordPrompt& sessionKeyPassword,
    OTPassword& newKeyPassword,
    const PasswordPrompt& reason) const
{
    auto dhPublicKey = Data::Factory();
    const auto havePubkey = publicKey.GetKey(dhPublicKey);

    if (!havePubkey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get public key.")
            .Flush();

        return false;
    }

    auto dhPrivateKey = api.Factory().BinarySecret();

    OT_ASSERT(dhPrivateKey);

    const bool havePrivateKey =
        AsymmetricKeyToECPrivatekey(api, privateKey, reason, *dhPrivateKey);

    if (!havePrivateKey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get private key.")
            .Flush();

        return false;
    }

    // Calculate ECDH shared secret
    const bool haveECDH = ECDH(dhPublicKey, *dhPrivateKey, newKeyPassword);

    if (!haveECDH) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": ECDH shared secret negotiation failed.")
            .Flush();

        return false;
    }

    const bool encrypted =
        sessionKey.ChangePassword(sessionKeyPassword, newKeyPassword);

    if (!encrypted) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Session key encryption failed.")
            .Flush();

        return false;
    }

    return true;
}

bool EcdsaProvider::ExportECPrivatekey(
    const api::Core& api,
    const OTPassword& privkey,
    const PasswordPrompt& reason,
    crypto::key::EllipticCurve& asymmetricKey) const
{
    std::unique_ptr<proto::Ciphertext> encryptedKey(new proto::Ciphertext);

    EncryptPrivateKey(api, privkey, reason, *encryptedKey);

    return asymmetricKey.SetKey(encryptedKey);
}

bool EcdsaProvider::ImportECPrivatekey(
    const api::Core& api,
    const proto::Ciphertext& asymmetricKey,
    const PasswordPrompt& reason,
    OTPassword& privkey) const
{
    return DecryptPrivateKey(api, asymmetricKey, reason, privkey);
}

bool EcdsaProvider::PrivateToPublic(
    const api::Core& api,
    const proto::AsymmetricKey& privateKey,
    proto::AsymmetricKey& publicKey,
    const PasswordPrompt& reason) const
{
    publicKey.CopyFrom(privateKey);
    publicKey.clear_chaincode();
    publicKey.clear_key();
    publicKey.set_mode(proto::KEYMODE_PUBLIC);
    auto key = Data::Factory();

    if (false == PrivateToPublic(api, privateKey.encryptedkey(), key, reason)) {

        return false;
    }

    publicKey.set_key(key->data(), key->size());

    return true;
}

bool EcdsaProvider::PrivateToPublic(
    const api::Core& api,
    const proto::Ciphertext& privateKey,
    Data& publicKey,
    const PasswordPrompt& reason) const
{
    auto plaintextKey = api.Factory().BinarySecret();

    OT_ASSERT(plaintextKey);

    const bool decrypted =
        DecryptPrivateKey(api, privateKey, reason, *plaintextKey);

    if (!decrypted) { return false; }

    return ScalarBaseMultiply(*plaintextKey, publicKey);
}

bool EcdsaProvider::SeedToCurveKey(
    [[maybe_unused]] const OTPassword& seed,
    [[maybe_unused]] OTPassword& privateKey,
    [[maybe_unused]] Data& publicKey) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": This provider does not support curve25519.")
        .Flush();

    return false;
}
}  // namespace opentxs::crypto::implementation
