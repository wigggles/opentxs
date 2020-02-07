// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/crypto/key/HD.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/key/Secp256k1.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/key/Keypair.hpp"

#include "crypto/library/AsymmetricProviderNull.hpp"

#include <memory>

namespace opentxs::crypto::key::implementation
{
class NullKeypair final : virtual public key::Keypair
{
public:
    operator bool() const noexcept final { return false; }

    bool CheckCapability(const NymCapability&) const noexcept final
    {
        return {};
    }
    const Asymmetric& GetPrivateKey() const noexcept(false) final
    {
        throw std::runtime_error("");
    }
    const Asymmetric& GetPublicKey() const noexcept(false) final
    {
        throw std::runtime_error("");
    }
    std::int32_t GetPublicKeyBySignature(Keys&, const Signature&, bool) const
        noexcept final
    {
        return {};
    }
    std::shared_ptr<proto::AsymmetricKey> GetSerialized(bool) const
        noexcept final
    {
        return {};
    }
    bool GetTransportKey(Data&, OTPassword&, const PasswordPrompt&) const
        noexcept final
    {
        return {};
    }

    NullKeypair* clone() const final { return new NullKeypair; }

    ~NullKeypair() final = default;
};

class Null : virtual public key::Asymmetric
{
public:
    std::unique_ptr<Asymmetric> asPublic() const noexcept final { return {}; }
    OTData CalculateHash(const proto::HashType, const PasswordPrompt&) const
        noexcept final
    {
        return Data::Factory();
    }
    bool CalculateID(Identifier&) const noexcept final { return false; }
    bool CalculateTag(
        const identity::Authority&,
        const proto::AsymmetricKeyType,
        const PasswordPrompt&,
        std::uint32_t&,
        OTPassword&) const noexcept final
    {
        return false;
    }
    bool CalculateTag(
        const Asymmetric&,
        const Identifier&,
        const PasswordPrompt&,
        std::uint32_t&) const noexcept final
    {
        return false;
    }
    bool CalculateSessionPassword(
        const Asymmetric&,
        const PasswordPrompt&,
        OTPassword&) const noexcept final
    {
        return false;
    }
    const opentxs::crypto::AsymmetricProvider& engine() const noexcept final
    {
        abort();
    }
    const OTSignatureMetadata* GetMetadata() const noexcept final
    {
        return nullptr;
    }
    bool hasCapability(const NymCapability&) const noexcept final
    {
        return false;
    }
    bool HasPrivate() const noexcept final { return false; }
    bool HasPublic() const noexcept final { return false; }
    proto::AsymmetricKeyType keyType() const noexcept final
    {
        return proto::AKEYTYPE_NULL;
    }
    ReadView Params() const noexcept final { return {}; }
    const std::string Path() const noexcept final { return {}; }
    bool Path(proto::HDPath&) const noexcept final { return false; }
    ReadView PrivateKey(const PasswordPrompt&) const noexcept final
    {
        return {};
    }
    ReadView PublicKey() const noexcept final { return {}; }
    proto::KeyRole Role() const noexcept final { return {}; }
    std::shared_ptr<proto::AsymmetricKey> Serialize() const noexcept final
    {
        return nullptr;
    }
    proto::HashType SigHashType() const noexcept final
    {
        return proto::HASHTYPE_NONE;
    }
    bool Sign(
        const GetPreimage,
        const proto::SignatureRole,
        proto::Signature&,
        const Identifier&,
        const PasswordPrompt&,
        proto::KeyRole,
        const proto::HashType) const noexcept final
    {
        return false;
    }
    bool TransportKey(Data&, OTPassword&, const PasswordPrompt&) const
        noexcept final
    {
        return false;
    }
    bool Verify(const Data&, const proto::Signature&) const noexcept final
    {
        return false;
    }
    VersionNumber Version() const noexcept final { return {}; }

    operator bool() const noexcept override { return false; }
    bool operator==(const proto::AsymmetricKey&) const noexcept final
    {
        return false;
    }

    Null() = default;
    ~Null() override = default;

private:
    Null* clone() const noexcept override { return new Null; }
};

class NullEC : virtual public key::EllipticCurve, public Null
{
public:
    operator bool() const noexcept final { return false; }

    std::unique_ptr<EllipticCurve> asPublicEC() const noexcept final
    {
        return {};
    }

    NullEC() = default;
    ~NullEC() override = default;
};

class NullHD : virtual public key::HD, public NullEC
{
public:
    ReadView Chaincode(const PasswordPrompt&) const noexcept final
    {
        return {};
    }
    int Depth() const noexcept final { return {}; }
    Bip32Fingerprint Fingerprint() const noexcept final { return {}; }
    std::string Xprv(const PasswordPrompt&) const noexcept final { return {}; }
    std::string Xpub(const PasswordPrompt&) const noexcept final { return {}; }

    NullHD() = default;
    ~NullHD() override = default;

private:
    NullHD* clone() const noexcept override { return new NullHD; }
};

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
class NullSecp256k1 final : virtual public key::Secp256k1, public NullHD
{
public:
    NullSecp256k1() = default;
    ~NullSecp256k1() final = default;

private:
    NullSecp256k1* clone() const noexcept final { return new NullSecp256k1; }
};
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
}  // namespace opentxs::crypto::key::implementation
