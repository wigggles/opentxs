// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_HDSEED_HPP
#define OPENTXS_API_HDSEED_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_WITH_BIP39
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace opentxs
{
namespace api
{
class HDSeed
{
public:
    using Path = std::vector<Bip32Index>;

    EXPORT virtual std::unique_ptr<opentxs::crypto::key::HD> AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual std::string Bip32Root(
        const PasswordPrompt& reason,
        const std::string& fingerprint = "") const = 0;
    EXPORT virtual std::string DefaultSeed() const = 0;
    EXPORT virtual std::unique_ptr<opentxs::crypto::key::HD> GetHDKey(
        std::string& fingerprint,
        const EcdsaCurve& curve,
        const Path& path,
        const PasswordPrompt& reason,
        const proto::KeyRole role = proto::KEYROLE_SIGN,
        const VersionNumber version =
            opentxs::crypto::key::EllipticCurve::DefaultVersion) const = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> GetPaymentCode(
        std::string& fingerprint,
        const Bip32Index nym,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual OTSymmetricKey GetStorageKey(
        std::string& seed,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual std::string ImportRaw(
        const OTPassword& entropy,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual std::string ImportSeed(
        const OTPassword& words,
        const OTPassword& passphrase,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual std::string NewSeed(const PasswordPrompt& reason) const = 0;
    EXPORT virtual std::string Passphrase(
        const PasswordPrompt& reason,
        const std::string& fingerprint = "") const = 0;
    EXPORT virtual std::shared_ptr<OTPassword> Seed(
        std::string& fingerprint,
        Bip32Index& index,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual bool UpdateIndex(
        std::string& seed,
        const Bip32Index index,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual std::string Words(
        const PasswordPrompt& reason,
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
