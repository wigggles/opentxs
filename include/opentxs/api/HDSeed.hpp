// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_HDSEED_HPP
#define OPENTXS_API_HDSEED_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_WITH_BIP39
#include "opentxs/Proto.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{
namespace api
{
class HDSeed
{
public:
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const Bip32Index index) const = 0;
    EXPORT virtual std::string Bip32Root(
        const std::string& fingerprint = "") const = 0;
    EXPORT virtual std::string DefaultSeed() const = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> GetPaymentCode(
        std::string& fingerprint,
        const Bip32Index nym) const = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> GetStorageKey(
        std::string& seed) const = 0;
    EXPORT virtual std::string ImportSeed(
        const OTPassword& words,
        const OTPassword& passphrase) const = 0;
    EXPORT virtual std::string NewSeed() const = 0;
    EXPORT virtual std::string Passphrase(
        const std::string& fingerprint = "") const = 0;
    EXPORT virtual std::shared_ptr<OTPassword> Seed(
        std::string& fingerprint,
        Bip32Index& index) const = 0;
    EXPORT virtual bool UpdateIndex(std::string& seed, const Bip32Index index)
        const = 0;
    EXPORT virtual std::string Words(
        const std::string& fingerprint = "") const = 0;

    EXPORT virtual ~HDSeed() = default;

protected:
    HDSeed() = default;

private:
    HDSeed(const HDSeed&) = delete;
    HDSeed(HDSeed&&) = delete;
    HDSeed& operator=(const HDSeed&) = delete;
    HDSeed& operator=(HDSeed&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif  // OT_CRYPTO_WITH_BIP39
#endif
