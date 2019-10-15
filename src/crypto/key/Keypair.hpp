// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::crypto::key::implementation
{
class Keypair final : virtual public key::Keypair
{
public:
    operator bool() const noexcept final { return true; }

    bool CheckCapability(const NymCapability& capability) const noexcept final;
    const Asymmetric& GetPrivateKey() const noexcept(false) final;
    const Asymmetric& GetPublicKey() const noexcept(false) final;
    std::int32_t GetPublicKeyBySignature(
        Keys& listOutput,
        const Signature& theSignature,
        bool bInclusive = false) const noexcept final;
    std::shared_ptr<proto::AsymmetricKey> GetSerialized(
        bool privateKey = false) const noexcept final;
    bool GetTransportKey(
        Data& publicKey,
        OTPassword& privateKey,
        const PasswordPrompt& reason) const noexcept final;

    ~Keypair() final = default;

private:
    friend opentxs::LowLevelKeyGenerator;
    friend key::Keypair;
    friend opentxs::Factory;

    const api::internal::Core& api_;
    OTAsymmetricKey m_pkeyPrivate;
    OTAsymmetricKey m_pkeyPublic;
    const proto::KeyRole role_{proto::KEYROLE_ERROR};

    Keypair* clone() const final { return new Keypair(*this); }

    bool make_new_keypair(const NymParameters& nymParameters);

    Keypair(
        const api::internal::Core& api,
        const proto::AsymmetricKey& serializedPubkey,
        const PasswordPrompt& reason) noexcept;
    Keypair(
        const api::internal::Core& api,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const proto::KeyRole role,
        const PasswordPrompt& reason) noexcept;
    Keypair(
        const api::internal::Core& api,
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey,
        const PasswordPrompt& reason) noexcept;
    Keypair() = delete;
    Keypair(const Keypair&) noexcept;
    Keypair(Keypair&&) = delete;
    Keypair& operator=(const Keypair&) = delete;
    Keypair& operator=(Keypair&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
