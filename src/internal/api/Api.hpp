// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/Context.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"

namespace
{
/** Callbacks in this form allow OpenSSL to query opentxs to get key encryption
 *  and decryption passwords*/
extern "C" {
typedef std::int32_t INTERNAL_PASSWORD_CALLBACK(
    char* buf,
    std::int32_t size,
    std::int32_t rwflag,
    void* userdata);
}
}  // namespace

namespace opentxs::internal
{
}  // namespace opentxs::internal

namespace opentxs::api::internal
{
struct Context : virtual public api::Context {
    virtual auto GetPasswordCaller() const -> OTCaller& = 0;
    virtual void Init() = 0;
    virtual auto Legacy() const noexcept -> const api::Legacy& = 0;
    virtual void shutdown() = 0;

    virtual ~Context() = default;
};

struct Core : virtual public api::Core {
    const proto::Ciphertext encrypted_secret_{};

    virtual auto GetInternalPasswordCallback() const
        -> INTERNAL_PASSWORD_CALLBACK* = 0;
    virtual auto GetSecret(
        const opentxs::Lock& lock,
        OTPassword& secret,
        const PasswordPrompt& reason,
        const bool twice) const -> bool = 0;
    virtual auto Legacy() const noexcept -> const api::Legacy& = 0;
    virtual auto Lock() const -> std::mutex& = 0;
    virtual auto MasterKey(const opentxs::Lock& lock) const
        -> const opentxs::crypto::key::Symmetric& = 0;

    virtual ~Core() = default;
};

struct Factory : virtual public api::Factory {
    virtual auto Asymmetric() const
        -> const api::crypto::internal::Asymmetric& = 0;
    virtual auto Symmetric() const -> const api::crypto::Symmetric& = 0;

    virtual ~Factory() = default;
};

struct Log {
    virtual ~Log() = default;
};
}  // namespace opentxs::api::internal
