// Copyright (c) 2018 The Open-Transactions developers
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
    bool CheckCapability(const NymCapability& capability) const override;
    const Asymmetric& GetPrivateKey() const override;
    const Asymmetric& GetPublicKey() const override;
    std::int32_t GetPublicKeyBySignature(
        Keys& listOutput,
        const Signature& theSignature,
        bool bInclusive = false) const override;
    std::shared_ptr<proto::AsymmetricKey> GetSerialized(
        bool privateKey = false) const override;
    bool GetTransportKey(Data& publicKey, OTPassword& privateKey)
        const override;

    ~Keypair() = default;

private:
    friend opentxs::LowLevelKeyGenerator;
    friend key::Keypair;
    friend opentxs::Factory;

    OTAsymmetricKey m_pkeyPublic;
    OTAsymmetricKey m_pkeyPrivate;
    const proto::KeyRole role_{proto::KEYROLE_ERROR};

    Keypair* clone() const override final { return new Keypair(*this); }

    bool make_new_keypair(const NymParameters& nymParameters);

    Keypair(
        const api::Core& api,
        const proto::AsymmetricKey& serializedPubkey) noexcept;
    Keypair(
        const api::Core& api,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const proto::KeyRole role = proto::KEYROLE_ERROR) noexcept;
    Keypair(
        const api::Core& api,
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey) noexcept;
    Keypair() = delete;
    Keypair(const Keypair&) noexcept;
    Keypair(Keypair&&) = delete;
    Keypair& operator=(const Keypair&) = delete;
    Keypair& operator=(Keypair&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
