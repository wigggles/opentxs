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
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"

namespace opentxs
{
namespace api
{
class Factory;

namespace crypto
{
class Asymmetric;
class Symmetric;
}  // namespace crypto

namespace storage
{
class Storage;
}  // namespace storage
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

class Factory;
class OTPassword;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::implementation
{
class HDSeed final : public api::HDSeed
{
public:
#if OT_CRYPTO_WITH_BIP32
    std::unique_ptr<opentxs::crypto::key::HD> AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const final;
#endif  // OT_CRYPTO_WITH_BIP32
    std::string Bip32Root(
        const PasswordPrompt& reason,
        const std::string& fingerprint = "") const final;
    std::string DefaultSeed() const final;
#if OT_CRYPTO_WITH_BIP32
    std::unique_ptr<opentxs::crypto::key::HD> GetHDKey(
        std::string& fingerprint,
        const EcdsaCurve& curve,
        const Path& path,
        const PasswordPrompt& reason,
        const proto::KeyRole role = proto::KEYROLE_SIGN,
        const VersionNumber version =
            opentxs::crypto::key::EllipticCurve::DefaultVersion) const final;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    std::unique_ptr<opentxs::crypto::key::Secp256k1> GetPaymentCode(
        std::string& fingerprint,
        const Bip32Index nym,
        const PasswordPrompt& reason) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    OTSymmetricKey GetStorageKey(
        std::string& seed,
        const PasswordPrompt& reason) const final;
#endif  // OT_CRYPTO_WITH_BIP32
    std::string ImportRaw(
        const OTPassword& entropy,
        const PasswordPrompt& reason) const final;
    std::string ImportSeed(
        const OTPassword& words,
        const OTPassword& passphrase,
        const PasswordPrompt& reason) const final;
    std::string NewSeed(const PasswordPrompt& reason) const final;
    std::string Passphrase(
        const PasswordPrompt& reason,
        const std::string& fingerprint = "") const final;
    std::shared_ptr<OTPassword> Seed(
        std::string& fingerprint,
        Bip32Index& index,
        const PasswordPrompt& reason) const final;
    bool UpdateIndex(
        std::string& seed,
        const Bip32Index index,
        const PasswordPrompt& reason) const final;
    std::string Words(
        const PasswordPrompt& reason,
        const std::string& fingerprint = "") const final;

    virtual ~HDSeed() = default;

private:
    friend opentxs::Factory;

    static const std::string DEFAULT_PASSPHRASE;
    static const proto::SymmetricMode DEFAULT_ENCRYPTION_MODE;
    static const VersionNumber DefaultVersion{3};
    static const OTPassword binary_secret_;
    static const OTPassword text_secret_;

    const api::Factory& factory_;
    const api::crypto::Asymmetric& asymmetric_;
    const api::crypto::Symmetric& symmetric_;
    const api::storage::Storage& storage_;
    const opentxs::crypto::Bip32& bip32_;
    const opentxs::crypto::Bip39& bip39_;

    bool decrypt_seed(
        const proto::Seed& seed,
        OTPassword& words,
        OTPassword& phrase,
        OTPassword& raw,
        const PasswordPrompt& reason) const;
    std::string save_seed(
        const OTPassword& words,
        const OTPassword& passphrase,
        const OTPassword& raw,
        const PasswordPrompt& reason) const;
    bool seed_to_data(
        const OTPassword& words,
        const OTPassword& passphrase,
        const OTPassword& raw,
        OTPassword& output) const;
    std::shared_ptr<proto::Seed> serialized_seed(
        std::string& fingerprint,
        Bip32Index& index,
        const PasswordPrompt& reason) const;

    HDSeed(
        const api::Factory& factory,
        const api::crypto::Asymmetric& asymmetric,
        const api::crypto::Symmetric& symmetric,
        const api::storage::Storage& storage,
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39);
    HDSeed() = delete;
    HDSeed(const HDSeed&) = delete;
    HDSeed(HDSeed&&) = delete;
    HDSeed& operator=(const HDSeed&) = delete;
    HDSeed& operator=(HDSeed&&) = delete;
};
}  // namespace opentxs::api::implementation
