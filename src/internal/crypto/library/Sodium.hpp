// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/crypto/Util.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/library/HashingProvider.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"
#include "internal/crypto/library/Scrypt.hpp"

namespace opentxs::crypto
{
class Sodium : virtual public api::crypto::Util
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    ,
               virtual public EcdsaProvider
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
    ,
               virtual public HashingProvider,
               virtual public SymmetricProvider,
               virtual public Scrypt
{
public:
    ~Sodium() override = default;

protected:
    Sodium() = default;

private:
    Sodium(const Sodium&) = delete;
    Sodium(Sodium&&) = delete;
    auto operator=(const Sodium&) -> Sodium& = delete;
    auto operator=(Sodium &&) -> Sodium& = delete;
};
}  // namespace opentxs::crypto
