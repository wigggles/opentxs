// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>

#include "crypto/key/Asymmetric.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key

class EcdsaProvider;
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
class Ciphertext;
class HDPath;
}  // namespace proto

class Data;
class OTPassword;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::crypto::key::implementation
{
class EllipticCurve : virtual public key::EllipticCurve, public Asymmetric
{
public:
    operator bool() const noexcept final { return Asymmetric::operator bool(); }

    std::unique_ptr<key::Asymmetric> asPublic() const noexcept final
    {
        return asPublicEC();
    }
    std::unique_ptr<key::EllipticCurve> asPublicEC() const noexcept final;
    virtual NymParameterType CreateType() const = 0;
    const std::string Path() const noexcept override { return {}; }
    bool Path(proto::HDPath&) const noexcept override { return {}; }

    virtual ~EllipticCurve() override = default;

protected:
    const crypto::EcdsaProvider& ecdsa_;

    static std::shared_ptr<proto::AsymmetricKey> serialize_public(
        EllipticCurve* copy);

    virtual EllipticCurve* clone_ec() const = 0;
    virtual std::shared_ptr<proto::AsymmetricKey> get_public() const = 0;

    EllipticCurve(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey) noexcept(false);
    EllipticCurve(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role,
        const VersionNumber version,
        const PasswordPrompt& reason) noexcept(false);
#if OT_CRYPTO_WITH_BIP32
    EllipticCurve(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKeyType keyType,
        const OTPassword& privateKey,
        const Data& publicKey,
        const proto::KeyRole role,
        const VersionNumber version,
        key::Symmetric& sessionKey,
        const PasswordPrompt& reason) noexcept(false);
#endif  // OT_CRYPTO_WITH_BIP32
    EllipticCurve(const EllipticCurve&) noexcept;

private:
    friend class crypto::EcdsaProvider;

    static auto extract_key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serialized,
        Data& publicKey) -> std::unique_ptr<proto::Ciphertext>;

    EllipticCurve() = delete;
    EllipticCurve(EllipticCurve&&) = delete;
    EllipticCurve& operator=(const EllipticCurve&) = delete;
    EllipticCurve& operator=(EllipticCurve&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
