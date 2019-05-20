// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::implementation
{
class HDSeed final : public api::HDSeed
{
public:
    std::shared_ptr<proto::AsymmetricKey> AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const Bip32Index index) const override;
    std::string Bip32Root(const std::string& fingerprint = "") const override;
    std::string DefaultSeed() const override;
    std::shared_ptr<proto::AsymmetricKey> GetPaymentCode(
        std::string& fingerprint,
        const Bip32Index nym) const override;
    std::shared_ptr<proto::AsymmetricKey> GetStorageKey(
        std::string& seed) const override;
    std::string ImportSeed(
        const OTPassword& words,
        const OTPassword& passphrase) const override;
    std::string NewSeed() const override;
    std::string Passphrase(const std::string& fingerprint = "") const override;
    std::shared_ptr<OTPassword> Seed(
        std::string& fingerprint,
        Bip32Index& index) const override;
    bool UpdateIndex(std::string& seed, const Bip32Index index) const override;
    std::string Words(const std::string& fingerprint = "") const override;

    virtual ~HDSeed() = default;

private:
    friend opentxs::Factory;

    static const std::string DEFAULT_PASSPHRASE;
    static const proto::SymmetricMode DEFAULT_ENCRYPTION_MODE;

    const api::crypto::Symmetric& symmetric_;
    const api::storage::Storage& storage_;
    const opentxs::crypto::Bip32& bip32_;
    const opentxs::crypto::Bip39& bip39_;
    const opentxs::crypto::LegacySymmetricProvider& aes_;

    bool DecryptSeed(
        const proto::Seed& seed,
        OTPassword& words,
        OTPassword& phrase) const;
    std::string SaveSeed(const OTPassword& words, const OTPassword& passphrase)
        const;
    bool SeedToData(
        const OTPassword& words,
        const OTPassword& passphrase,
        OTPassword& output) const;
    std::shared_ptr<proto::Seed> SerializedSeed(
        std::string& fingerprint,
        Bip32Index& index) const;

    HDSeed(
        const api::crypto::Symmetric& symmetric,
        const api::storage::Storage& storage,
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39,
        const opentxs::crypto::LegacySymmetricProvider& aes);
    HDSeed() = delete;
    HDSeed(const HDSeed&) = delete;
    HDSeed(HDSeed&&) = delete;
    HDSeed& operator=(const HDSeed&) = delete;
    HDSeed& operator=(HDSeed&&) = delete;
};
}  // namespace opentxs::api::implementation
