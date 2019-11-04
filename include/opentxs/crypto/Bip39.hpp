// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_BIP39_HPP
#define OPENTXS_CRYPTO_BIP39_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_WITH_BIP39
#include "opentxs/Proto.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{
namespace crypto
{
class Bip39
{
public:
    OPENTXS_EXPORT virtual bool SeedToWords(
        const OTPassword& seed,
        OTPassword& words) const = 0;
    OPENTXS_EXPORT virtual void WordsToSeed(
        const OTPassword& words,
        OTPassword& seed,
        const OTPassword& passphrase) const = 0;

    OPENTXS_EXPORT virtual ~Bip39() = default;

protected:
    Bip39() = default;

private:
    Bip39(const Bip39&) = delete;
    Bip39(Bip39&&) = delete;
    Bip39& operator=(const Bip39&) = delete;
    Bip39& operator=(Bip39&&) = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif  // OT_CRYPTO_WITH_BIP39
#endif
