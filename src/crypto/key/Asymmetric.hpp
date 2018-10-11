// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::crypto::key::implementation
{
class Asymmetric : virtual public key::Asymmetric
{
public:
    static key::Asymmetric* KeyFactory(
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role);

    /** Only works for public keys. */
    bool CalculateID(Identifier& theOutput) const override;
    const OTSignatureMetadata* GetMetadata() const override
    {
        return m_pMetadata;
    }
    bool hasCapability(const NymCapability& capability) const override;
    bool IsPrivate() const override { return m_bIsPrivateKey; }
    bool IsPublic() const override { return m_bIsPublicKey; }
    proto::AsymmetricKeyType keyType() const override;
    const std::string Path() const override;
    bool Path(proto::HDPath& output) const override;
    const proto::KeyRole& Role() const override { return role_; }
    std::shared_ptr<proto::AsymmetricKey> Serialize() const override;
    OTData SerializeKeyToData(const proto::AsymmetricKey& rhs) const override;
    proto::HashType SigHashType() const override { return StandardHash; }
    bool Sign(
        const Data& plaintext,
        proto::Signature& sig,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr,
        const String& credID = String::Factory(""),
        const proto::SignatureRole role = proto::SIGROLE_ERROR) const override;
    bool Verify(const Data& plaintext, const proto::Signature& sig)
        const override;

    void Release() override;
    void ReleaseKey() override { Release(); }
    /** Don't use this, normally it's not necessary. */
    void SetAsPublic() override
    {
        m_bIsPublicKey = true;
        m_bIsPrivateKey = false;
    }
    /** (Only if you really know what you are doing.) */
    void SetAsPrivate() override
    {
        m_bIsPublicKey = false;
        m_bIsPrivateKey = true;
    }

    operator bool() const override;
    bool operator==(const proto::AsymmetricKey&) const override;

    virtual ~Asymmetric();

protected:
    proto::AsymmetricKeyType m_keyType{proto::AKEYTYPE_ERROR};
    proto::KeyRole role_{proto::KEYROLE_ERROR};
    bool m_bIsPublicKey{false};
    bool m_bIsPrivateKey{false};
    Timer m_timer;

    explicit Asymmetric(const proto::AsymmetricKey& serializedKey);
    Asymmetric(
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role);
    Asymmetric();

protected:
    friend OTAsymmetricKey;

    // To use m_metadata, call m_metadata.HasMetadata(). If it's true, then you
    // can see these values:
    //    char m_metadata::Getproto::AsymmetricKeyType()             // Can be
    //    A, E, or S
    //    (authentication, encryption, or signing. Also, E would be unusual.)
    //    char m_metadata::FirstCharNymID()         // Can be any letter from
    //    base62 alphabet. Represents first letter of a Nym's ID.
    //    char m_metadata::FirstCharMasterCredID()  // Can be any letter from
    //    base62 alphabet. Represents first letter of a Master Credential ID
    //    (for that Nym.)
    //    char m_metadata::FirstCharChildCredID()     // Can be any letter from
    //    base62 alphabet. Represents first letter of a Credential ID (signed by
    //    that Master.)
    //
    // Here's how metadata works: It's optional. You can set it, or not. If it's
    // there, OT will add it to the signature on the contract itself, when this
    // key is used to sign something. (Signature has the same
    // OTSignatureMetadata struct.) Later on when verifying the signature, the
    // metadata is used to speed up the lookup/verification process so we don't
    // have to verify the signature against every single child key credential
    // available for that Nym. In practice, however, we are adding metadata to
    // every single signature (except possibly cash...) (And we will make it
    // mandatory for Nyms who use credentials.)
    OTSignatureMetadata* m_pMetadata{nullptr};

    virtual Asymmetric* clone() const override;

    virtual void ReleaseKeyLowLevel_Hook() {}

    Asymmetric(
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role,
        const bool publicKey,
        const bool privateKey);
    Asymmetric(const Asymmetric&) = delete;
    Asymmetric(Asymmetric&&) = delete;
    Asymmetric& operator=(const Asymmetric&) = delete;
    Asymmetric& operator=(Asymmetric&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
