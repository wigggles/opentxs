// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "crypto/library/AsymmetricProviderNull.hpp"

#include <memory>

namespace opentxs::crypto::key::implementation
{
class Null : virtual public key::Asymmetric
{
public:
    OTData CalculateHash(
        const proto::HashType hashType,
        const OTPasswordData& password) const override
    {
        return Data::Factory();
    }
    bool CalculateID(Identifier&) const override { return false; }
    const opentxs::crypto::AsymmetricProvider& engine() const override
    {
        throw;
    }
    const OTSignatureMetadata* GetMetadata() const override { return nullptr; }
    bool GetPublicKey(String&) const override { return false; }
    bool hasCapability(const NymCapability&) const override { return false; }
    bool IsEmpty() const override { return false; }
    bool IsPrivate() const override { return false; }
    bool IsPublic() const override { return false; }
    proto::AsymmetricKeyType keyType() const override
    {
        return proto::AKEYTYPE_NULL;
    }
    bool Open(
        crypto::key::Asymmetric&,
        crypto::key::Symmetric&,
        OTPasswordData&) const override
    {
        return false;
    }
    const std::string Path() const override { return {}; }
    bool Path(proto::HDPath&) const override { return false; }
    bool ReEncryptPrivateKey(const OTPassword&, bool bImporting) const override
    {
        return false;
    }
    const proto::KeyRole& Role() const override { throw; }
    std::shared_ptr<proto::AsymmetricKey> Serialize() const override
    {
        return nullptr;
    }
    OTData SerializeKeyToData(const proto::AsymmetricKey&) const override
    {
        return Data::Factory();
    }
    proto::HashType SigHashType() const override
    {
        return proto::HASHTYPE_NONE;
    }
    bool Sign(
        const Data&,
        proto::Signature&,
        const OTPasswordData* = nullptr,
        const OTPassword* = nullptr,
        const String& = String::Factory(""),
        const proto::SignatureRole = proto::SIGROLE_ERROR) const override
    {
        return false;
    }
    bool Sign(
        const GetPreimage,
        const proto::SignatureRole,
        proto::Signature&,
        const Identifier&,
        proto::KeyRole key,
        const OTPasswordData* pPWData,
        const proto::HashType hash) const override
    {
        return false;
    }
    bool TransportKey(Data&, OTPassword&) const override { return false; }
    bool Verify(const Data&, const proto::Signature&) const override
    {
        return false;
    }

    void Release() override {}
    void ReleaseKey() override {}
    bool Seal(
        const opentxs::api::Core&,
        OTAsymmetricKey&,
        crypto::key::Symmetric&,
        OTPasswordData&) const override
    {
        return false;
    }
    void SetAsPublic() override {}
    void SetAsPrivate() override {}

    operator bool() const override { return false; }
    bool operator==(const proto::AsymmetricKey&) const override
    {
        return false;
    }

    Null() = default;
    ~Null() = default;

private:
    Null* clone() const override { return new Null; }

    Null(const Null&) = delete;
    Null(Null&&) = delete;
    Null& operator=(const Null&) = delete;
    Null& operator=(Null&&) = delete;
};

#if OT_CRYPTO_SUPPORTED_KEY_HD
class NullEC : virtual public key::EllipticCurve, public Null
{
public:
    const crypto::EcdsaProvider& ECDSA() const override { throw; }
    bool GetKey(Data&) const override { return {}; }
    bool GetKey(proto::Ciphertext&) const override { return {}; }
    using Asymmetric::GetPublicKey;
    bool GetPublicKey(Data&) const override { return {}; }
    OTData PrivateKey() const override { return Data::Factory(); }
    OTData PublicKey() const override { return Data::Factory(); }

    bool SetKey(const Data&) override { return {}; }
    bool SetKey(std::unique_ptr<proto::Ciphertext>&) override { return {}; }

    NullEC() = default;
    ~NullEC() = default;

private:
    NullEC(const NullEC&) = delete;
    NullEC(NullEC&&) = delete;
    NullEC& operator=(const NullEC&) = delete;
    NullEC& operator=(NullEC&&) = delete;
};

class NullHD : virtual public key::HD, public NullEC
{
public:
    OTData Chaincode() const override { return Data::Factory(); }
    int Depth() const override { return {}; }
    Bip32Fingerprint Fingerprint() const override { return {}; }
    std::string Xprv() const override { return {}; }
    std::string Xpub() const override { return {}; }

    NullHD() = default;
    ~NullHD() = default;

private:
    NullHD(const NullHD&) = delete;
    NullHD(NullHD&&) = delete;
    NullHD& operator=(const NullHD&) = delete;
    NullHD& operator=(NullHD&&) = delete;
};
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
}  // namespace opentxs::crypto::key::implementation
