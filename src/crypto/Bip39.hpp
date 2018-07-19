// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef IMPLEMENTATION_OPENTXS_CRYPTO_BIP39_HPP
#define IMPLEMENTATION_OPENTXS_CRYPTO_BIP39_HPP

#include "opentxs/crypto/Bip39.hpp"

namespace opentxs::crypto::implementation
{
class Bip39 : virtual public crypto::Bip39
{
public:
    std::string DefaultSeed() const override;
    std::string ImportSeed(
        const OTPassword& words,
        const OTPassword& passphrase) const override;
    std::string NewSeed() const override;
    std::string Passphrase(const std::string& fingerprint = "") const override;
    std::shared_ptr<OTPassword> Seed(
        std::string& fingerprint,
        std::uint32_t& index) const override;
    bool UpdateIndex(std::string& seed, const std::uint32_t index)
        const override;
    std::string Words(const std::string& fingerprint = "") const override;

    virtual ~Bip39() = default;

protected:
    const api::Native& native_;

    Bip39(const api::Native& native);

private:
    static const std::string DEFAULT_PASSPHRASE;
    static const proto::SymmetricMode DEFAULT_ENCRYPTION_MODE;

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
        std::uint32_t& index) const;

    virtual bool toWords(const OTPassword& seed, OTPassword& words) const = 0;
    virtual void WordsToSeed(
        const OTPassword& words,
        OTPassword& seed,
        const OTPassword& passphrase) const = 0;

    Bip39() = delete;
    Bip39(const Bip39&) = delete;
    Bip39(Bip39&&) = delete;
    Bip39& operator=(const Bip39&) = delete;
    Bip39& operator=(Bip39&&) = delete;
};
}  // namespace opentxs::crypto::implementation
#endif  // IMPLEMENTATION_OPENTXS_CRYPTO_BIP39_HPP
