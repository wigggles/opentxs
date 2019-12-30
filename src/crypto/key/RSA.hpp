// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/key/RSA.hpp"

#include "Asymmetric.hpp"

#include <memory>

#if OT_CRYPTO_SUPPORTED_KEY_RSA
namespace opentxs::crypto::key::implementation
{
class RSA final : public key::RSA, public Asymmetric
{
public:
    std::unique_ptr<key::Asymmetric> asPublic() const noexcept final;
    ReadView Params() const noexcept final { return params_->Bytes(); }
    std::shared_ptr<proto::AsymmetricKey> Serialize() const noexcept final;
    proto::HashType SigHashType() const noexcept final
    {
        return proto::HASHTYPE_SHA256;
    }

    RSA(const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::KeyRole role,
        const VersionNumber version,
        const NymParameters& options,
        Space& params,
        const PasswordPrompt& reason)
    noexcept(false);
    RSA(const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKey& serialized)
    noexcept(false);
    RSA(const RSA&) noexcept;

    ~RSA() final = default;

private:
    const OTData params_;

    static auto deserialize_key(
        const api::internal::Core& api,
        const proto::AsymmetricKey& serialized,
        Data& publicKey,
        OTPassword& privateKey) noexcept(false)
        -> std::unique_ptr<proto::Ciphertext>;

    RSA* clone() const noexcept final { return new RSA{*this}; }

    RSA() = delete;
    RSA(RSA&&) = delete;
    RSA& operator=(const RSA&) = delete;
    RSA& operator=(RSA&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
