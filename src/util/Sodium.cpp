// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"

extern "C" {
#include <sodium.h>
}

#include <array>

#include "Sodium.hpp"

namespace opentxs::crypto::sodium
{
auto ExpandSeed(
    const OTPassword& seed,
    OTPassword& privateKey,
    Data& publicKey) noexcept -> bool
{
    if (!seed.isMemory()) { return false; }

    if (crypto_sign_SEEDBYTES != seed.getMemorySize()) { return false; }

    std::array<unsigned char, crypto_sign_SECRETKEYBYTES> secretKeyBlank{};
    privateKey.setMemory(secretKeyBlank.data(), secretKeyBlank.size());
    std::array<unsigned char, crypto_sign_PUBLICKEYBYTES> publicKeyBlank{};
    const auto output = ::crypto_sign_seed_keypair(
        publicKeyBlank.data(),
        static_cast<unsigned char*>(privateKey.getMemoryWritable()),
        static_cast<const unsigned char*>(seed.getMemory()));
    publicKey.Assign(publicKeyBlank.data(), publicKeyBlank.size());

    return (0 == output);
}
}  // namespace opentxs::crypto::sodium
