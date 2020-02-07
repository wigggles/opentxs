// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_PBKDF2_HPP
#define OPENTXS_CRYPTO_LIBRARY_PBKDF2_HPP

#include "Internal.hpp"

#include <cstdint>

namespace opentxs::crypto
{
class Pbkdf2
{
public:
    OPENTXS_EXPORT virtual bool PKCS5_PBKDF2_HMAC(
        const void* input,
        const std::size_t inputSize,
        const void* salt,
        const std::size_t saltSize,
        const std::size_t iterations,
        const proto::HashType hashType,
        const std::size_t bytes,
        void* output) const noexcept = 0;

    virtual ~Pbkdf2() = default;

protected:
    Pbkdf2() = default;

private:
    Pbkdf2(const Pbkdf2&) = delete;
    Pbkdf2(Pbkdf2&&) = delete;
    Pbkdf2& operator=(const Pbkdf2&) = delete;
    Pbkdf2& operator=(Pbkdf2&&) = delete;
};
}  // namespace opentxs::crypto
#endif
