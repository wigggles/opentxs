// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/LegacySymmetricProvider.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"
#include "opentxs/core/Log.hpp"

#include <string>

#include "Symmetric.hpp"

#define OT_METHOD "opentxs::api::crypto::implementation::Symmetric::"

namespace opentxs
{
api::crypto::Symmetric* Factory::Symmetric(crypto::SymmetricProvider& sodium)
{
    return new api::crypto::implementation::Symmetric(sodium);
}
}  // namespace opentxs

namespace opentxs::api::crypto::implementation
{
Symmetric::Symmetric(opentxs::crypto::SymmetricProvider& sodium)
    : sodium_(sodium)
{
}

opentxs::crypto::SymmetricProvider* Symmetric::GetEngine(
    const proto::SymmetricMode mode) const
{
    opentxs::crypto::SymmetricProvider* engine = nullptr;

    // Add support for other crypto engines here
    switch (mode) {
        default: {
            engine = &sodium_;
        }
    }

    return engine;
}

std::size_t Symmetric::IvSize(const proto::SymmetricMode mode) const
{
    auto pEngine = GetEngine(mode);

    if (nullptr == pEngine) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid mode").Flush();

        return 0;
    }

    auto& engine = *pEngine;

    return engine.IvSize(mode);
}

OTSymmetricKey Symmetric::Key(
    const OTPasswordData& password,
    const proto::SymmetricMode mode) const
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return opentxs::crypto::key::Symmetric::Factory(*engine, password, mode);
}

OTSymmetricKey Symmetric::Key(
    const proto::SymmetricKey& serialized,
    const proto::SymmetricMode mode) const
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return opentxs::crypto::key::Symmetric::Factory(*engine, serialized);
}

OTSymmetricKey Symmetric::Key(
    const OTPassword& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const proto::SymmetricMode mode,
    const proto::SymmetricKeyType type) const
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return opentxs::crypto::key::Symmetric::Factory(
        *engine, seed, operations, difficulty, engine->KeySize(mode), type);
}
}  // namespace opentxs::api::crypto::implementation
