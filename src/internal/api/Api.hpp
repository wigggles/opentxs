// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Native.hpp"

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
struct Core : virtual public api::Core {
    const proto::Ciphertext encrypted_secret_{};

    virtual INTERNAL_PASSWORD_CALLBACK* GetInternalPasswordCallback() const = 0;
    virtual bool GetSecret(
        const opentxs::Lock& lock,
        OTPassword& secret,
        const PasswordPrompt& reason,
        const bool twice) const = 0;
    virtual std::mutex& Lock() const = 0;
    virtual const opentxs::crypto::key::Symmetric& MasterKey(
        const opentxs::Lock& lock) const = 0;

    virtual ~Core() = default;
};

struct Native : virtual public api::Native {
    virtual OTCaller& GetPasswordCaller() const = 0;
    virtual void Init() = 0;
    virtual void shutdown() = 0;

    virtual ~Native() = default;
};

struct Factory : virtual public api::Factory {
    virtual const api::crypto::internal::Asymmetric& Asymmetric() const = 0;
    virtual const api::crypto::Symmetric& Symmetric() const = 0;

    virtual ~Factory() = default;
};

struct Log {
    virtual ~Log() = default;
};
}  // namespace opentxs::api::internal
