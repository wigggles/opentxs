// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_MKCERT_HPP
#define OPENTXS_CORE_CRYPTO_MKCERT_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_RSA

#include <cstdint>

extern "C" {
#include <openssl/x509v3.h>

std::int32_t mkcert(
    X509** x509p,
    EVP_PKEY** pkeyp,
    std::int32_t bits,
    std::int32_t serial,
    std::int32_t days);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#endif
