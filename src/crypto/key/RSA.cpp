// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/Log.hpp"

#include "crypto/library/OpenSSL.hpp"
#include "internal/api/Api.hpp"

#include "RSA.hpp"

extern "C" {
#include <openssl/pem.h>
}

#if OT_CRYPTO_SUPPORTED_KEY_RSA
namespace opentxs
{
using ReturnType = crypto::key::implementation::RSA;

auto Factory::RSAKey(
    const api::internal::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& input) noexcept
    -> std::unique_ptr<crypto::key::RSA>
{
    try {
        return std::make_unique<ReturnType>(api, engine, input);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::RSAKey(
    const api::internal::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::KeyRole role,
    const VersionNumber version,
    const NymParameters& options,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::RSA>
{
    try {
        auto params = Space{};

        return std::make_unique<ReturnType>(
            api, engine, role, version, options, params, reason);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs

namespace opentxs::crypto::key::implementation
{
RSA::RSA(
    const api::internal::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& serialized) noexcept(false)
    : Asymmetric(
          api,
          engine,
          serialized,
          [&](auto& pub, auto& prv) -> EncryptedKey {
              return deserialize_key(api, serialized, pub, prv);
          })
    , params_(api_.Factory().Data(serialized.params()))
{
}

RSA::RSA(
    const api::internal::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::KeyRole role,
    const VersionNumber version,
    const NymParameters& options,
    Space& params,
    const PasswordPrompt& reason) noexcept(false)
    : Asymmetric(
          api,
          engine,
          proto::AKEYTYPE_LEGACY,
          role,
          version,
          [&](auto& pub, auto& prv) -> EncryptedKey {
              return create_key(
                  api,
                  engine,
                  options,
                  role,
                  pub.WriteInto(),
                  prv.WriteInto(),
                  prv,
                  writer(params),
                  reason);
          })
    , params_(api_.Factory().Data(params))
{
    if (false == bool(encrypted_key_)) {
        throw std::runtime_error("Failed to instantiate encrypted_key_");
    }
}

RSA::RSA(const RSA& rhs) noexcept
    : Asymmetric(rhs)
    , params_(rhs.params_)
{
}

auto RSA::asPublic() const noexcept -> std::unique_ptr<key::Asymmetric>
{
    auto output = std::make_unique<RSA>(*this);

    OT_ASSERT(output);

    auto& copy = *output;
    copy.erase_private_data();

    OT_ASSERT(false == copy.HasPrivate());

    return std::move(output);
}

auto RSA::deserialize_key(
    const api::internal::Core& api,
    const proto::AsymmetricKey& proto,
    Data& publicKey,
    OTPassword&) noexcept(false) -> std::unique_ptr<proto::Ciphertext>
{
    auto output = std::unique_ptr<proto::Ciphertext>{};
    publicKey.Assign(proto.key());

    if (proto.has_encryptedkey()) {
        output = std::make_unique<proto::Ciphertext>(proto.encryptedkey());

        OT_ASSERT(output);
    }

    return output;
}

auto RSA::Serialize() const noexcept -> std::shared_ptr<proto::AsymmetricKey>
{
    auto output = Asymmetric::Serialize();

    OT_ASSERT(output)

    if (proto::KEYROLE_ENCRYPT == role_) {
        output->set_params(params_->data(), params_->size());
    }

    return output;
}
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
