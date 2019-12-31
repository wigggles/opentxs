// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/crypto/Signature.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

extern "C" {
#include <sodium/crypto_box.h>
}

#include "EllipticCurve.hpp"

// #define OT_METHOD "opentxs::crypto::key::implementation::EllipticCurve::"

namespace opentxs::crypto::key
{
const VersionNumber EllipticCurve::DefaultVersion{2};
const VersionNumber EllipticCurve::MaxVersion{2};
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
EllipticCurve::EllipticCurve(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serialized) noexcept(false)
    : Asymmetric(
          api,
          ecdsa,
          serialized,
          [&](auto& pubkey, auto&) -> EncryptedKey {
              return extract_key(api, ecdsa, serialized, pubkey);
          })
    , ecdsa_(ecdsa)
{
}

EllipticCurve::EllipticCurve(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role,
    const VersionNumber version,
    const PasswordPrompt& reason) noexcept(false)
    : Asymmetric(
          api,
          ecdsa,
          keyType,
          role,
          version,
          [&](auto& pub, auto& prv) -> EncryptedKey {
              return create_key(
                  api,
                  ecdsa,
                  {},
                  role,
                  pub.WriteInto(),
                  prv.WriteInto(OTPassword::Mode::Mem),
                  prv,
                  {},
                  reason);
          })
    , ecdsa_(ecdsa)
{
    if (false == bool(encrypted_key_)) {
        throw std::runtime_error("Failed to instantiate encrypted_key_");
    }
}

#if OT_CRYPTO_WITH_BIP32
EllipticCurve::EllipticCurve(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKeyType keyType,
    const OTPassword& privateKey,
    const Data& publicKey,
    const proto::KeyRole role,
    const VersionNumber version,
    key::Symmetric& sessionKey,
    const PasswordPrompt& reason) noexcept(false)
    : Asymmetric(
          api,
          ecdsa,
          keyType,
          role,
          true,
          true,
          version,
          OTData{publicKey},
          [&](auto&, auto&) -> EncryptedKey {
              return encrypt_key(sessionKey, reason, true, privateKey.Bytes());
          })
    , ecdsa_(ecdsa)
{
    if (false == bool(encrypted_key_)) {
        throw std::runtime_error("Failed to instantiate encrypted_key_");
    }
}
#endif  // OT_CRYPTO_WITH_BIP32

EllipticCurve::EllipticCurve(const EllipticCurve& rhs) noexcept
    : Asymmetric(rhs)
    , ecdsa_(rhs.ecdsa_)
{
}

auto EllipticCurve::asPublicEC() const noexcept
    -> std::unique_ptr<key::EllipticCurve>
{
    auto output = std::unique_ptr<EllipticCurve>{clone_ec()};

    OT_ASSERT(output);

    auto& copy = *output;
    copy.erase_private_data();

    OT_ASSERT(false == copy.HasPrivate());

    return std::move(output);
}

auto EllipticCurve::extract_key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& proto,
    Data& publicKey) -> std::unique_ptr<proto::Ciphertext>
{
    auto output = std::unique_ptr<proto::Ciphertext>{};
    publicKey.Assign(proto.key());

    if ((proto::KEYMODE_PRIVATE == proto.mode()) && proto.has_encryptedkey()) {
        output = std::make_unique<proto::Ciphertext>(proto.encryptedkey());

        OT_ASSERT(output);
    }

    return output;
}

auto EllipticCurve::serialize_public(EllipticCurve* in)
    -> std::shared_ptr<proto::AsymmetricKey>
{
    std::unique_ptr<EllipticCurve> copy{in};

    OT_ASSERT(copy);

    copy->erase_private_data();

    return copy->Serialize();
}
}  // namespace opentxs::crypto::key::implementation
