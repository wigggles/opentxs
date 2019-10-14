// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_SECP256K1_HPP
#define OPENTXS_CRYPTO_LIBRARY_SECP256K1_HPP

#include "Internal.hpp"

#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"

namespace opentxs::crypto
{
class Secp256k1 : virtual public EcdsaProvider
{
public:
    EXPORT virtual void Init() = 0;

    EXPORT ~Secp256k1() override = default;

protected:
    Secp256k1() = default;

private:
    Secp256k1(const Secp256k1&) = delete;
    Secp256k1(Secp256k1&&) = delete;
    Secp256k1& operator=(const Secp256k1&) = delete;
    Secp256k1& operator=(Secp256k1&&) = delete;
};
}  // namespace opentxs::crypto
#endif  // OT_CRYPTO_USING_LIBSECP256K1
#endif
