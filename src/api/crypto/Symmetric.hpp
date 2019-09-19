// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::crypto::implementation
{
class Symmetric final : virtual public api::crypto::Symmetric
{
public:
    virtual std::size_t IvSize(const proto::SymmetricMode mode) const final;
    OTSymmetricKey Key(
        const PasswordPrompt& password,
        const proto::SymmetricMode mode =
            proto::SMODE_CHACHA20POLY1305) const final;
    OTSymmetricKey Key(
        const proto::SymmetricKey& serialized,
        const proto::SymmetricMode mode) const final;
    OTSymmetricKey Key(
        const OTPassword& seed,
        const std::uint64_t operations = 0,
        const std::uint64_t difficulty = 0,
        const proto::SymmetricMode mode = proto::SMODE_CHACHA20POLY1305,
        const proto::SymmetricKeyType type =
            proto::SKEYTYPE_ARGON2) const final;

    ~Symmetric() final = default;

private:
    friend opentxs::Factory;

    const api::Core& api_;

    const opentxs::crypto::SymmetricProvider* GetEngine(
        const proto::SymmetricMode mode) const;

    Symmetric(const api::Core& api);
    Symmetric() = delete;
    Symmetric(const Symmetric&) = delete;
    Symmetric(Symmetric&&) = delete;
    Symmetric& operator=(const Symmetric&) = delete;
    Symmetric& operator=(Symmetric&&) = delete;
};
}  // namespace opentxs::api::crypto::implementation
