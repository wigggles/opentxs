// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/crypto/Bip32.hpp"
#endif
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Ed25519.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/crypto/key/RSA.hpp"
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/key/Secp256k1.hpp"
#endif
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/Types.hpp"

#include "Null.hpp"

#include <cstdint>
#include <cstring>
#include <memory>
#include <ostream>
#include <string>

#include "Asymmetric.hpp"

template class opentxs::Pimpl<opentxs::crypto::key::Asymmetric>;

#define OT_METHOD "opentxs::crypto::key::implementation::Asymmetric::"

namespace opentxs::crypto::key
{
const VersionNumber Asymmetric::DefaultVersion{1};
const VersionNumber Asymmetric::MaxVersion{2};

OTAsymmetricKey Asymmetric::Factory()
{
    return OTAsymmetricKey(new implementation::Null);
}
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
const std::map<proto::SignatureRole, VersionNumber> Asymmetric::sig_version_{
    {proto::SIGROLE_PUBCREDENTIAL, 1},
    {proto::SIGROLE_PRIVCREDENTIAL, 1},
    {proto::SIGROLE_NYMIDSOURCE, 1},
    {proto::SIGROLE_CLAIM, 1},
    {proto::SIGROLE_SERVERCONTRACT, 1},
    {proto::SIGROLE_UNITDEFINITION, 1},
    {proto::SIGROLE_PEERREQUEST, 1},
    {proto::SIGROLE_PEERREPLY, 1},
    {proto::SIGROLE_CONTEXT, 2},
    {proto::SIGROLE_ACCOUNT, 2},
    {proto::SIGROLE_SERVERREQUEST, 3},
    {proto::SIGROLE_SERVERREPLY, 3},
};

Asymmetric::Asymmetric(
    const api::internal::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role,
    const bool hasPublic,
    const bool hasPrivate,
    const VersionNumber version) noexcept
    : key::Asymmetric()
    , api_(api)
    , provider_(engine)
    , version_(version)
    , type_(keyType)
    , role_(role)
    , has_public_(hasPublic)
    , has_private_(hasPrivate)
    , m_timer()
    , m_pMetadata(new OTSignatureMetadata)
{
    OT_ASSERT(0 < version);
    OT_ASSERT(nullptr != m_pMetadata);
}

Asymmetric::Asymmetric(
    const api::internal::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& key) noexcept
    : Asymmetric(
          api,
          engine,
          key.type(),
          key.role(),
          proto::KEYMODE_PUBLIC == key.mode(),
          proto::KEYMODE_PRIVATE == key.mode(),
          key.version())
{
}

Asymmetric::Asymmetric(
    const api::internal::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& serialized,
    const bool hasPublic,
    const bool hasPrivate) noexcept
    : Asymmetric(
          api,
          engine,
          serialized.type(),
          serialized.role(),
          hasPublic,
          hasPrivate,
          serialized.version())
{
}

Asymmetric::Asymmetric(
    const api::internal::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role,
    const VersionNumber version) noexcept
    : Asymmetric(api, engine, keyType, role, false, false, version)
{
}

Asymmetric::Asymmetric(const Asymmetric& rhs) noexcept
    : Asymmetric(
          rhs.api_,
          rhs.provider_,
          rhs.type_,
          rhs.role_,
          rhs.has_public_,
          rhs.has_private_,
          rhs.version_)
{
}

Asymmetric::operator bool() const { return has_public_ || has_private_; }

bool Asymmetric::operator==(const proto::AsymmetricKey& rhs) const
{
    std::shared_ptr<proto::AsymmetricKey> tempKey = Serialize();
    auto LHData = SerializeKeyToData(*tempKey);
    auto RHData = SerializeKeyToData(rhs);

    return (LHData == RHData);
}

bool Asymmetric::CalculateID(Identifier& theOutput) const
{
    theOutput.Release();

    if (false == HasPublic()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing public key").Flush();

        return false;
    }

    auto strPublicKey = String::Factory();
    bool bGotPublicKey = get_public_key(strPublicKey);

    if (!bGotPublicKey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error getting public key.")
            .Flush();
        return false;
    }

    bool bSuccessCalculateDigest = theOutput.CalculateDigest(strPublicKey);

    if (!bSuccessCalculateDigest) {
        theOutput.Release();
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error calculating digest of public key.")
            .Flush();
        return false;
    }

    return true;
}

bool Asymmetric::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::SIGN_CHILDCRED): {
        }
        case (NymCapability::SIGN_MESSAGE): {
        }
        case (NymCapability::ENCRYPT_MESSAGE): {

            return true;
        }
        default: {
        }
    }

    return false;
}

proto::AsymmetricKeyType Asymmetric::keyType() const { return type_; }

proto::Signature Asymmetric::NewSignature(
    const Identifier& credentialID,
    const proto::SignatureRole role,
    const proto::HashType hash) const
{
    proto::Signature output{};
    output.set_version(sig_version_.at(role));
    output.set_credentialid(credentialID.str());
    output.set_role(role);
    output.set_hashtype((proto::HASHTYPE_ERROR == hash) ? SigHashType() : hash);
    output.clear_signature();

    return output;
}

const std::string Asymmetric::Path() const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key type.").Flush();

    return "";
}

bool Asymmetric::Path(proto::HDPath&) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key type.").Flush();

    return false;
}

void Asymmetric::Release()
{
    ReleaseKeyLowLevel_Hook();

    m_timer.clear();
}

std::shared_ptr<proto::AsymmetricKey> Asymmetric::Serialize() const

{
    auto serializedKey = std::make_shared<proto::AsymmetricKey>();
    serializedKey->set_version(version_);
    serializedKey->set_role(role_);
    serializedKey->set_type(static_cast<proto::AsymmetricKeyType>(type_));

    return serializedKey;
}

OTData Asymmetric::SerializeKeyToData(
    const proto::AsymmetricKey& serializedKey) const
{
    return proto::ProtoAsData(serializedKey);
}

bool Asymmetric::Sign(
    const Data& plaintext,
    proto::Signature& sig,
    const PasswordPrompt& reason,
    const OTPassword* exportPassword,
    const String& credID,
    const proto::SignatureRole role) const
{
    if (false == HasPrivate()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing private key").Flush();

        return false;
    }

    auto signature = Data::Factory();
    const auto hash = SigHashType();

    bool goodSig = engine().Sign(
        api_, plaintext, *this, hash, signature, reason, exportPassword);

    if (goodSig) {
        sig.set_version(1);
        if (credID.Exists()) { sig.set_credentialid(credID.Get()); }
        if (proto::SIGROLE_ERROR != role) { sig.set_role(role); }
        sig.set_hashtype(hash);
        sig.set_signature(signature->data(), signature->size());
    }

    return goodSig;
}

bool Asymmetric::Sign(
    const GetPreimage input,
    const proto::SignatureRole role,
    proto::Signature& signature,
    const Identifier& credential,
    const PasswordPrompt& reason,
    proto::KeyRole key,
    const proto::HashType hash) const
{
    if (false == HasPrivate()) {
        LogOutput(OT_METHOD)(__FUNCTION__)("Missing private key").Flush();

        return false;
    }

    try {
        signature = NewSignature(credential, role, hash);
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid signature role.").Flush();

        return false;
    }

    auto sig = Data::Factory();
    const auto raw = input();
    const auto preimage = Data::Factory(raw.data(), raw.size());
    bool goodSig = engine().Sign(
        api_, preimage, *this, signature.hashtype(), sig, reason, nullptr);

    if (goodSig) {
        signature.set_signature(sig->data(), sig->size());
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to sign preimate").Flush();
    }

    return goodSig;
}

bool Asymmetric::Verify(
    const Data& plaintext,
    const proto::Signature& sig,
    const PasswordPrompt& reason) const
{
    if (false == HasPublic()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing public key").Flush();

        return false;
    }

    auto signature = Data::Factory();
    signature->Assign(sig.signature().c_str(), sig.signature().size());

    return engine().Verify(plaintext, *this, signature, sig.hashtype(), reason);
}

Asymmetric::~Asymmetric()
{
    m_timer.clear();

    if (nullptr != m_pMetadata) { delete m_pMetadata; }

    m_pMetadata = nullptr;
}
}  // namespace opentxs::crypto::key::implementation
