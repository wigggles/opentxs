// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/crypto/library/EcdsaProvider.hpp"

namespace opentxs::crypto::implementation
{
class EcdsaProvider : virtual public crypto::EcdsaProvider
{
public:
    bool AsymmetricKeyToECPrivatekey(
        const api::Core& api,
        const crypto::key::EllipticCurve& asymmetricKey,
        const PasswordPrompt& reason,
        OTPassword& privkey) const override;
    bool DecryptSessionKeyECDH(
        const api::Core& api,
        const crypto::key::EllipticCurve& privateKey,
        const crypto::key::EllipticCurve& publicKey,
        crypto::key::Symmetric& sessionKey,
        PasswordPrompt& sessionKeyPassword,
        OTPassword& plaintextKey,
        const PasswordPrompt& reason) const override;
    bool DecryptSessionKeyECDH(
        const api::Core& api,
        const crypto::key::EllipticCurve& privateKey,
        const crypto::key::EllipticCurve& publicKey,
        crypto::key::Symmetric& sessionKey,
        PasswordPrompt& sessionKeyPassword,
        const PasswordPrompt& reason) const override;
    bool ECPrivatekeyToAsymmetricKey(
        const api::Core& api,
        const OTPassword& privkey,
        const PasswordPrompt& reason,
        crypto::key::EllipticCurve& asymmetricKey) const override;
    bool ECPubkeyToAsymmetricKey(
        const Data& pubkey,
        crypto::key::EllipticCurve& asymmetricKey) const override;
    bool EncryptSessionKeyECDH(
        const api::Core& api,
        const crypto::key::EllipticCurve& privateKey,
        const crypto::key::EllipticCurve& publicKey,
        crypto::key::Symmetric& sessionKey,
        const PasswordPrompt& sessionKeyPassword,
        OTPassword& newKeyPassword,
        const PasswordPrompt& reason) const override;
    bool ExportECPrivatekey(
        const api::Core& api,
        const OTPassword& privkey,
        const PasswordPrompt& reason,
        crypto::key::EllipticCurve& asymmetricKey) const override;
    bool ImportECPrivatekey(
        const api::Core& api,
        const proto::Ciphertext& asymmetricKey,
        const PasswordPrompt& reason,
        OTPassword& privkey) const override;
    bool PrivateToPublic(
        const api::Core& api,
        const proto::AsymmetricKey& privateKey,
        proto::AsymmetricKey& publicKey,
        const PasswordPrompt& reason) const override;
    bool PrivateToPublic(
        const api::Core& api,
        const proto::Ciphertext& privateKey,
        Data& publicKey,
        const PasswordPrompt& reason) const override;
    bool SeedToCurveKey(
        const OTPassword& seed,
        OTPassword& privateKey,
        Data& publicKey) const override;

    virtual ~EcdsaProvider() = default;

protected:
    const api::Crypto& crypto_;

    bool AsymmetricKeyToECPubkey(
        const crypto::key::EllipticCurve& asymmetricKey,
        Data& pubkey) const;
    bool AsymmetricKeyToECPrivkey(
        const api::Core& api,
        const proto::Ciphertext& asymmetricKey,
        const PasswordPrompt& reason,
        OTPassword& privkey) const;
    bool DecryptPrivateKey(
        const api::Core& api,
        const proto::Ciphertext& encryptedKey,
        const PasswordPrompt& reason,
        OTPassword& plaintextKey) const;
    bool DecryptPrivateKey(
        const api::Core& api,
        const proto::Ciphertext& encryptedKey,
        const proto::Ciphertext& encryptedChaincode,
        const PasswordPrompt& reason,
        OTPassword& key,
        OTPassword& chaincode) const;
    bool EncryptPrivateKey(
        const api::Core& api,
        const OTPassword& plaintextKey,
        const PasswordPrompt& reason,
        proto::Ciphertext& encryptedKey) const;
    bool EncryptPrivateKey(
        const api::Core& api,
        const OTPassword& key,
        const OTPassword& chaincode,
        const PasswordPrompt& reason,
        proto::Ciphertext& encryptedKey,
        proto::Ciphertext& encryptedChaincode) const;

    EcdsaProvider(const api::Crypto& crypto);

private:
    virtual bool ECDH(
        const Data& publicKey,
        const OTPassword& privateKey,
        OTPassword& secret) const = 0;
    virtual bool ScalarBaseMultiply(const OTPassword& seed, Data& publicKey)
        const = 0;

    EcdsaProvider() = delete;
    EcdsaProvider(const EcdsaProvider&) = delete;
    EcdsaProvider(EcdsaProvider&&) = delete;
    EcdsaProvider& operator=(const EcdsaProvider&) = delete;
    EcdsaProvider& operator=(EcdsaProvider&&) = delete;
};
}  // namespace opentxs::crypto::implementation
