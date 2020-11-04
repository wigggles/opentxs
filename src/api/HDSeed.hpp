// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/HDSeed.cpp"

#pragma once

#include <memory>
#include <string>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/protobuf/Enums.pb.h"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
class Symmetric;
}  // namespace crypto

namespace storage
{
class Storage;
}  // namespace storage

class Factory;
}  // namespace api

namespace crypto
{
namespace key
{
class HD;
class Secp256k1;
}  // namespace key

class Bip32;
class Bip39;
}  // namespace crypto

namespace proto
{
class HDPath;
class Seed;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::implementation
{
class HDSeed final : public api::HDSeed
{
public:
#if OT_CRYPTO_WITH_BIP32
    auto AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
#endif  // OT_CRYPTO_WITH_BIP32
    auto Bip32Root(
        const PasswordPrompt& reason,
        const std::string& fingerprint = "") const -> std::string final;
    auto DefaultSeed() const -> std::string final;
#if OT_CRYPTO_WITH_BIP32
    auto GetHDKey(
        std::string& fingerprint,
        const EcdsaCurve& curve,
        const Path& path,
        const PasswordPrompt& reason,
        const proto::KeyRole role = proto::KEYROLE_SIGN,
        const VersionNumber version =
            opentxs::crypto::key::EllipticCurve::DefaultVersion) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto GetPaymentCode(
        std::string& fingerprint,
        const Bip32Index nym,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto GetStorageKey(std::string& seed, const PasswordPrompt& reason) const
        -> OTSymmetricKey final;
#endif  // OT_CRYPTO_WITH_BIP32
    auto ImportRaw(const Secret& entropy, const PasswordPrompt& reason) const
        -> std::string final;
    auto ImportSeed(
        const Secret& words,
        const Secret& passphrase,
        const PasswordPrompt& reason) const -> std::string final;
    auto NewSeed(const PasswordPrompt& reason) const -> std::string final;
    auto Passphrase(
        const PasswordPrompt& reason,
        const std::string& fingerprint = "") const -> std::string final;
    auto Seed(
        std::string& fingerprint,
        Bip32Index& index,
        const PasswordPrompt& reason) const -> OTSecret final;
    auto UpdateIndex(
        std::string& seed,
        const Bip32Index index,
        const PasswordPrompt& reason) const -> bool final;
    auto Words(
        const PasswordPrompt& reason,
        const std::string& fingerprint = "") const -> std::string final;

    HDSeed(
        const api::Factory& factory,
        const api::crypto::Asymmetric& asymmetric,
        const api::crypto::Symmetric& symmetric,
        const api::storage::Storage& storage,
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39);

    virtual ~HDSeed() = default;

private:
    static const std::string DEFAULT_PASSPHRASE;
    static const proto::SymmetricMode DEFAULT_ENCRYPTION_MODE;
    static const VersionNumber DefaultVersion{3};

    const api::crypto::Symmetric& symmetric_;
#if OT_CRYPTO_WITH_BIP32
    const api::crypto::Asymmetric& asymmetric_;
#endif  // OT_CRYPTO_WITH_BIP32
    const api::storage::Storage& storage_;
    const opentxs::crypto::Bip32& bip32_;
    const opentxs::crypto::Bip39& bip39_;
    const OTSecret binary_secret_;
    const OTSecret text_secret_;

    auto decrypt_seed(
        const proto::Seed& seed,
        Secret& words,
        Secret& phrase,
        Secret& raw,
        const PasswordPrompt& reason) const -> bool;
    auto save_seed(
        const Secret& words,
        const Secret& passphrase,
        const Secret& raw,
        const PasswordPrompt& reason) const -> std::string;
    auto seed_to_data(
        const Secret& words,
        const Secret& passphrase,
        const Secret& raw,
        Secret& output) const -> bool;
    auto serialized_seed(
        std::string& fingerprint,
        Bip32Index& index,
        const PasswordPrompt& reason) const -> std::shared_ptr<proto::Seed>;

    HDSeed() = delete;
    HDSeed(const HDSeed&) = delete;
    HDSeed(HDSeed&&) = delete;
    auto operator=(const HDSeed&) -> HDSeed& = delete;
    auto operator=(HDSeed &&) -> HDSeed& = delete;
};
}  // namespace opentxs::api::implementation
