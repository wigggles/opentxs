// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/crypto/LowLevelKeyGenerator.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/crypto/Signature.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"

#include <ostream>

#include "Keypair.hpp"

#define OT_METHOD "opentxs::crypto::key::implementation::Keypair::"

template class opentxs::Pimpl<opentxs::crypto::key::Keypair>;

namespace opentxs
{
crypto::key::Keypair* Factory::Keypair(
    const api::Core& api,
    const NymParameters& nymParameters,
    const VersionNumber version,
    const proto::KeyRole role)
{
    return new crypto::key::implementation::Keypair(
        api, nymParameters, version, role);
}

crypto::key::Keypair* Factory::Keypair(
    const api::Core& api,
    const proto::AsymmetricKey& serializedPubkey,
    const proto::AsymmetricKey& serializedPrivkey)
{
    return new crypto::key::implementation::Keypair(
        api, serializedPubkey, serializedPrivkey);
}

crypto::key::Keypair* Factory::Keypair(
    const api::Core& api,
    const proto::AsymmetricKey& serializedPubkey)
{
    return new crypto::key::implementation::Keypair(api, serializedPubkey);
}
}  // namespace opentxs

namespace opentxs::crypto::key::implementation
{
Keypair::Keypair(
    const api::Core& api,
    const NymParameters& params,
    const VersionNumber version,
    const proto::KeyRole role) noexcept
    : m_pkeyPublic(api.Factory().AsymmetricKey(params, role, version))
    , m_pkeyPrivate(api.Factory().AsymmetricKey(params, role, version))
    , role_(role)
{
    const bool haveKeys = make_new_keypair(params);

    OT_ASSERT(haveKeys);
    OT_ASSERT(m_pkeyPublic.get());
    OT_ASSERT(m_pkeyPrivate.get());
}

Keypair::Keypair(
    const api::Core& api,
    const proto::AsymmetricKey& serializedPubkey,
    const proto::AsymmetricKey& serializedPrivkey) noexcept
    : m_pkeyPublic(api.Factory().AsymmetricKey(serializedPubkey))
    , m_pkeyPrivate(api.Factory().AsymmetricKey(serializedPrivkey))
    , role_(m_pkeyPrivate->Role())
{
    OT_ASSERT(m_pkeyPublic.get());
    OT_ASSERT(m_pkeyPrivate.get());
}

Keypair::Keypair(
    const api::Core& api,
    const proto::AsymmetricKey& serializedPubkey) noexcept
    : m_pkeyPublic(api.Factory().AsymmetricKey(serializedPubkey))
    , m_pkeyPrivate(Asymmetric::Factory())
    , role_(m_pkeyPublic->Role())
{
    OT_ASSERT(m_pkeyPublic.get());
    OT_ASSERT(false == bool(m_pkeyPrivate.get()));
}

Keypair::Keypair(const Keypair& rhs) noexcept
    : key::Keypair()
    , m_pkeyPublic(rhs.m_pkeyPublic)
    , m_pkeyPrivate(rhs.m_pkeyPrivate)
    , role_(rhs.role_)
{
}

bool Keypair::CheckCapability(const NymCapability& capability) const
{
    bool output{false};

    if (m_pkeyPrivate.get()) {
        output |= m_pkeyPrivate->hasCapability(capability);
    } else if (m_pkeyPublic.get()) {
        output |= m_pkeyPublic->hasCapability(capability);
    }

    return output;
}

// Return the private key as an Asymmetric object
const Asymmetric& Keypair::GetPrivateKey() const
{
    if (m_pkeyPrivate.get()) { return m_pkeyPrivate; }

    throw std::runtime_error("not a private key");
}

// Return the public key as an Asymmetric object
const Asymmetric& Keypair::GetPublicKey() const
{
    if (m_pkeyPublic.get()) { return m_pkeyPublic; }

    throw std::runtime_error("not a private key");
}

std::int32_t Keypair::GetPublicKeyBySignature(
    Keys& listOutput,  // Inclusive means, return the key even
                       // when theSignature has no metadata.
    const Signature& theSignature,
    bool bInclusive) const
{
    OT_ASSERT(m_pkeyPublic.get());

    const auto* metadata = m_pkeyPublic->GetMetadata();

    OT_ASSERT(nullptr != metadata);

    // We know that EITHER exact metadata matches must occur, and the signature
    // MUST have metadata, (bInclusive=false)
    // OR if bInclusive=true, we know that metadata is still used to eliminate
    // keys where possible, but that otherwise,
    // if the signature has no metadata, then the key is still returned, "just
    // in case."
    //
    if ((false == bInclusive) &&
        (false == theSignature.getMetaData().HasMetadata()))
        return 0;

    // Below this point, metadata is used if it's available.
    // It's assumed to be "okay" if it's not available, since any non-inclusive
    // calls would have already returned by now, if that were the case.
    // (But if it IS available, then it must match, or the key won't be
    // returned.)
    //
    // If the signature has no metadata, or if m_pkeyPublic has no metadata, or
    // if they BOTH have metadata, and their metadata is a MATCH...
    if (!theSignature.getMetaData().HasMetadata() || !metadata->HasMetadata() ||
        (metadata->HasMetadata() && theSignature.getMetaData().HasMetadata() &&
         (theSignature.getMetaData() == *(metadata)))) {
        // ...Then add m_pkeyPublic as a possible match, to listOutput.
        //
        listOutput.push_back(&m_pkeyPublic.get());
        return 1;
    }
    return 0;
}

std::shared_ptr<proto::AsymmetricKey> Keypair::GetSerialized(
    bool privateKey) const
{
    OT_ASSERT(m_pkeyPublic.get());

    if (privateKey) {
        OT_ASSERT(m_pkeyPrivate.get());

        return m_pkeyPrivate->Serialize();
    } else {
        return m_pkeyPublic->Serialize();
    }
}

bool Keypair::GetTransportKey(Data& publicKey, OTPassword& privateKey) const
{
    return m_pkeyPrivate->TransportKey(publicKey, privateKey);
}

bool Keypair::make_new_keypair(const NymParameters& nymParameters)
{
    LowLevelKeyGenerator lowLevelKeys(nymParameters);

    if (!lowLevelKeys.MakeNewKeypair()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed in a call to LowLevelKeyGenerator::MakeNewKeypair.")
            .Flush();
        return false;
    }

    OTPasswordData passwordData("Enter or set the wallet master password.");

    return lowLevelKeys.SetOntoKeypair(*this, passwordData);
}
}  // namespace opentxs::crypto::key::implementation
