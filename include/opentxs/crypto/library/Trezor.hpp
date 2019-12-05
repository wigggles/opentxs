// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_TREZOR_HPP
#define OPENTXS_CRYPTO_LIBRARY_TREZOR_HPP

#include "Internal.hpp"

#if OT_CRYPTO_USING_TREZOR
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/EncodingProvider.hpp"
#include "opentxs/crypto/library/Ripemd160.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/crypto/Bip32.hpp"
#endif

namespace opentxs::crypto
{
class Trezor : virtual public EncodingProvider,
               virtual public Ripemd160
#if OT_CRYPTO_WITH_BIP32
    ,
               virtual public Bip32
#endif
{
public:
    OPENTXS_EXPORT ~Trezor() override = default;

protected:
    Trezor() = default;

private:
    Trezor(const Trezor&) = delete;
    Trezor(Trezor&&) = delete;
    Trezor& operator=(const Trezor&) = delete;
    Trezor& operator=(Trezor&&) = delete;
};
}  // namespace opentxs::crypto
#endif  // OT_CRYPTO_USING_TREZOR
#endif
