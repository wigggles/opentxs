// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "api/crypto/Symmetric.hpp"  // IWYU pragma: associated

#include "Factory.hpp"
#include "internal/api/Api.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"

#define OT_METHOD "opentxs::api::crypto::implementation::Symmetric::"

namespace opentxs
{
api::crypto::Symmetric* Factory::Symmetric(const api::internal::Core& api)
{
    return new api::crypto::implementation::Symmetric(api);
}
}  // namespace opentxs

namespace opentxs::api::crypto::implementation
{
Symmetric::Symmetric(const api::internal::Core& api)
    : api_(api)
{
}

const opentxs::crypto::SymmetricProvider* Symmetric::GetEngine(
    const proto::SymmetricMode mode) const
{
    const opentxs::crypto::SymmetricProvider* engine{nullptr};

    // Add support for other crypto engines here
    switch (mode) {
        default: {
            engine = &api_.Crypto().Sodium();
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
    const PasswordPrompt& password,
    const proto::SymmetricMode mode) const
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return api_.Factory().SymmetricKey(*engine, password, mode);
}

OTSymmetricKey Symmetric::Key(
    const proto::SymmetricKey& serialized,
    const proto::SymmetricMode mode) const
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return api_.Factory().SymmetricKey(*engine, serialized);
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

    return api_.Factory().SymmetricKey(
        *engine, seed, operations, difficulty, engine->KeySize(mode), type);
}
}  // namespace opentxs::api::crypto::implementation
