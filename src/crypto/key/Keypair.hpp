// Copyright (c) 2010-2020 The Open-Transactions developers
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

    Keypair(
        const api::internal::Core& api,
        const proto::KeyRole role,
        std::unique_ptr<crypto::key::Asymmetric> publicKey,
        std::unique_ptr<crypto::key::Asymmetric> privateKey) noexcept;

    ~Keypair() final = default;

private:
    friend key::Keypair;

    const api::internal::Core& api_;
    OTAsymmetricKey m_pkeyPrivate;
    OTAsymmetricKey m_pkeyPublic;
    const proto::KeyRole role_;

    Keypair* clone() const final { return new Keypair(*this); }

    Keypair() = delete;
    Keypair(const Keypair&) noexcept;
    Keypair(Keypair&&) = delete;
    Keypair& operator=(const Keypair&) = delete;
    Keypair& operator=(Keypair&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
