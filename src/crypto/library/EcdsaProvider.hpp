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
        const crypto::key::EllipticCurve& asymmetricKey,
        const OTPasswordData& passwordData,
        OTPassword& privkey) const override;
    bool DecryptSessionKeyECDH(
        const crypto::key::EllipticCurve& privateKey,
        const crypto::key::EllipticCurve& publicKey,
        const OTPasswordData& password,
        crypto::key::Symmetric& sessionKey,
        OTPassword& plaintextKey) const override;
    bool DecryptSessionKeyECDH(
        const crypto::key::EllipticCurve& privateKey,
        const crypto::key::EllipticCurve& publicKey,
        const OTPasswordData& password,
        crypto::key::Symmetric& sessionKey) const override;
    bool ECPrivatekeyToAsymmetricKey(
        const OTPassword& privkey,
        const OTPasswordData& passwordData,
        crypto::key::EllipticCurve& asymmetricKey) const override;
    bool ECPubkeyToAsymmetricKey(
        const Data& pubkey,
        crypto::key::EllipticCurve& asymmetricKey) const override;
    bool EncryptSessionKeyECDH(
        const crypto::key::EllipticCurve& privateKey,
        const crypto::key::EllipticCurve& publicKey,
        const OTPasswordData& passwordData,
        crypto::key::Symmetric& sessionKey,
        OTPassword& newKeyPassword) const override;
    bool ExportECPrivatekey(
        const OTPassword& privkey,
        const OTPasswordData& password,
        crypto::key::EllipticCurve& asymmetricKey) const override;
    bool ImportECPrivatekey(
        const proto::Ciphertext& asymmetricKey,
        const OTPasswordData& password,
        OTPassword& privkey) const override;
    bool PrivateToPublic(
        const proto::AsymmetricKey& privateKey,
        proto::AsymmetricKey& publicKey) const override;
    bool PrivateToPublic(const proto::Ciphertext& privateKey, Data& publicKey)
        const override;
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
        const proto::Ciphertext& asymmetricKey,
        const OTPasswordData& passwordData,
        OTPassword& privkey) const;
    bool DecryptPrivateKey(
        const proto::Ciphertext& encryptedKey,
        const OTPasswordData& password,
        OTPassword& plaintextKey) const;
    bool DecryptPrivateKey(
        const proto::Ciphertext& encryptedKey,
        const proto::Ciphertext& encryptedChaincode,
        const OTPasswordData& password,
        OTPassword& key,
        OTPassword& chaincode) const;
    bool EncryptPrivateKey(
        const OTPassword& plaintextKey,
        const OTPasswordData& password,
        proto::Ciphertext& encryptedKey) const;
    bool EncryptPrivateKey(
        const OTPassword& key,
        const OTPassword& chaincode,
        const OTPasswordData& password,
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
