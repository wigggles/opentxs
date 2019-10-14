// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "Asymmetric.hpp"

namespace opentxs::crypto::key::implementation
{
class EllipticCurve : virtual public key::EllipticCurve, public Asymmetric
{
public:
    operator bool() const noexcept final { return Asymmetric::operator bool(); }

    std::unique_ptr<key::EllipticCurve> asPublic(
        const PasswordPrompt& reason) const final;
    OTData CalculateHash(
        const proto::HashType hashType,
        const PasswordPrompt& password) const override;
    virtual NymParameterType CreateType() const = 0;
    const crypto::EcdsaProvider& ECDSA() const override { return ecdsa_; }
    bool GetKey(Data& key) const override;
    bool GetKey(proto::Ciphertext& key) const override;
    bool get_public_key(String& strKey) const override;
    bool Open(
        crypto::key::Asymmetric& dhPublic,
        crypto::key::Symmetric& sessionKey,
        PasswordPrompt& sessionKeyPassword,
        const PasswordPrompt& reason) const override;
    const std::string Path() const override { return {}; }
    bool Path(proto::HDPath&) const override { return {}; }
    OTData PrivateKey(const PasswordPrompt& reason) const override;
    OTData PublicKey(const PasswordPrompt& reason) const override;
    bool Seal(
        const opentxs::api::Core& api,
        OTAsymmetricKey& dhPublic,
        crypto::key::Symmetric& key,
        const PasswordPrompt& reason,
        PasswordPrompt& sessionPassword) const override;
    std::shared_ptr<proto::AsymmetricKey> Serialize() const override;
    bool TransportKey(
        Data& publicKey,
        OTPassword& privateKey,
        const PasswordPrompt& reason) const override;

    bool SetKey(const Data& key) override;
    bool SetKey(std::unique_ptr<proto::Ciphertext>& key) override;

    virtual ~EllipticCurve() override = default;

protected:
    const crypto::EcdsaProvider& ecdsa_;
    OTData key_;
    std::unique_ptr<proto::Ciphertext> encrypted_key_{nullptr};

    static std::unique_ptr<proto::Ciphertext> encrypt_key(
        key::Symmetric& sessionKey,
        const PasswordPrompt& reason,
        const bool attach,
        const OTPassword& plaintext) noexcept;
    static bool encrypt_key(
        const api::internal::Core& api,
        const PasswordPrompt& reason,
        const OTPassword& plaintext,
        proto::Ciphertext& ciphertext) noexcept;
    static std::shared_ptr<proto::AsymmetricKey> serialize_public(
        EllipticCurve* copy);

    virtual EllipticCurve* clone_ec() const = 0;
    virtual std::shared_ptr<proto::AsymmetricKey> get_public() const = 0;
    virtual void erase_private_data();

    EllipticCurve(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey,
        const PasswordPrompt& reason) noexcept;
    EllipticCurve(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role,
        const VersionNumber version,
        const PasswordPrompt& reason) noexcept(false);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    EllipticCurve(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKeyType keyType,
        const OTPassword& privateKey,
        const Data& publicKey,
        const proto::KeyRole role,
        const VersionNumber version,
        key::Symmetric& sessionKey,
        const PasswordPrompt& reason) noexcept;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    EllipticCurve(const EllipticCurve&) noexcept;

private:
    friend class crypto::EcdsaProvider;

    static bool encrypt_key(
        key::Symmetric& sessionKey,
        const PasswordPrompt& reason,
        const bool attach,
        const OTPassword& plaintext,
        proto::Ciphertext& ciphertext) noexcept;
    static std::unique_ptr<proto::Ciphertext> extract_key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serialized,
        Data& publicKey,
        const PasswordPrompt& reason);

    EllipticCurve() = delete;
    EllipticCurve(EllipticCurve&&) = delete;
    EllipticCurve& operator=(const EllipticCurve&) = delete;
    EllipticCurve& operator=(EllipticCurve&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
