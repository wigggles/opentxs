// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_ECDSAPROVIDER_HPP
#define OPENTXS_CRYPTO_LIBRARY_ECDSAPROVIDER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/Proto.hpp"

namespace opentxs
{
namespace crypto
{
class EcdsaProvider : virtual public AsymmetricProvider
{
public:
    EXPORT virtual bool AsymmetricKeyToECPrivatekey(
        const api::Core& api,
        const crypto::key::EllipticCurve& asymmetricKey,
        const PasswordPrompt& reason,
        OTPassword& privkey) const = 0;
    EXPORT virtual bool DecryptSessionKeyECDH(
        const api::Core& api,
        const crypto::key::EllipticCurve& privateKey,
        const crypto::key::EllipticCurve& publicKey,
        crypto::key::Symmetric& sessionKey,
        PasswordPrompt& sessionKeyPassword,
        OTPassword& plaintextKey,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual bool DecryptSessionKeyECDH(
        const api::Core& api,
        const crypto::key::EllipticCurve& privateKey,
        const crypto::key::EllipticCurve& publicKey,
        crypto::key::Symmetric& sessionKey,
        PasswordPrompt& sessionKeyPassword,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual bool ECPrivatekeyToAsymmetricKey(
        const api::Core& api,
        const OTPassword& privkey,
        const PasswordPrompt& reason,
        crypto::key::EllipticCurve& asymmetricKey) const = 0;
    EXPORT virtual bool ECPubkeyToAsymmetricKey(
        const Data& pubkey,
        crypto::key::EllipticCurve& asymmetricKey) const = 0;
    EXPORT virtual bool EncryptSessionKeyECDH(
        const api::Core& api,
        const crypto::key::EllipticCurve& privateKey,
        const crypto::key::EllipticCurve& publicKey,
        crypto::key::Symmetric& sessionKey,
        const PasswordPrompt& sessionKeyPassword,
        OTPassword& newKeyPassword,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual bool ExportECPrivatekey(
        const api::Core& api,
        const OTPassword& privkey,
        const PasswordPrompt& reason,
        crypto::key::EllipticCurve& asymmetricKey) const = 0;
    EXPORT virtual bool ImportECPrivatekey(
        const api::Core& api,
        const proto::Ciphertext& asymmetricKey,
        const PasswordPrompt& reason,
        OTPassword& privkey) const = 0;
    EXPORT virtual bool PrivateToPublic(
        const api::Core& api,
        const proto::AsymmetricKey& privateKey,
        proto::AsymmetricKey& publicKey,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual bool PrivateToPublic(
        const api::Core& api,
        const proto::Ciphertext& privateKey,
        Data& publicKey,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual bool RandomKeypair(OTPassword& privateKey, Data& publicKey)
        const = 0;
    EXPORT virtual bool SeedToCurveKey(
        const OTPassword& seed,
        OTPassword& privateKey,
        Data& publicKey) const = 0;

    EXPORT virtual ~EcdsaProvider() = default;

protected:
    EcdsaProvider() = default;

private:
    EcdsaProvider(const EcdsaProvider&) = delete;
    EcdsaProvider(EcdsaProvider&&) = delete;
    EcdsaProvider& operator=(const EcdsaProvider&) = delete;
    EcdsaProvider& operator=(EcdsaProvider&&) = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif
