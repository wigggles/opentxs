// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#include "internal/api/Api.hpp"

#include "Signable.hpp"

#define OT_METHOD "opentxs::contract::implementation::Signable::"

namespace opentxs::contract::implementation
{
Signable::Signable(
    const api::internal::Core& api,
    const Nym_p& nym,
    const VersionNumber version,
    const std::string& conditions,
    const std::string& alias,
    OTIdentifier&& id,
    Signatures&& signatures) noexcept
    : api_(api)
    , lock_()
    , nym_(nym)
    , version_(version)
    , conditions_(conditions)
    , id_(id)
    , signatures_(std::move(signatures))
    , alias_(alias)
{
}

Signable::Signable(
    const api::internal::Core& api,
    const Nym_p& nym,
    const VersionNumber version,
    const std::string& conditions,
    const std::string& alias) noexcept
    : Signable(
          api,
          nym,
          version,
          conditions,
          alias,
          api.Factory().Identifier(),
          {})
{
}

Signable::Signable(const Signable& rhs) noexcept
    : api_(rhs.api_)
    , lock_()
    , nym_(rhs.nym_)
    , version_(rhs.version_)
    , conditions_(rhs.conditions_)
    , id_(rhs.id_)
    , signatures_(rhs.signatures_)
    , alias_(rhs.alias_)
{
}

std::string Signable::Alias() const
{
    Lock lock(lock_);

    return alias_;
}

auto Signable::first_time_init(const Lock& lock) -> void
{
    const_cast<OTIdentifier&>(id_) = GetID(lock);

    if (id_->empty()) { throw std::runtime_error("Failed to calculate id"); }
}

bool Signable::CheckID(const Lock& lock) const { return (GetID(lock) == id_); }

OTIdentifier Signable::id(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    return id_;
}

OTIdentifier Signable::ID() const
{
    Lock lock(lock_);

    return id(lock);
}

auto Signable::init_serialized(const Lock& lock) noexcept(false) -> void
{
    const auto id = GetID(lock);

    if (id_.get() != id) {
        throw std::runtime_error("Calculated id does not match serialized id");
    }
}

Nym_p Signable::Nym() const { return nym_; }

void Signable::SetAlias(const std::string& alias)
{
    Lock lock(lock_);

    alias_ = alias;
}

const std::string& Signable::Terms() const
{
    Lock lock(lock_);

    return conditions_;
}

bool Signable::update_signature(const Lock& lock, const PasswordPrompt& reason)
{
    OT_ASSERT(verify_write_lock(lock));

    if (!nym_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing nym.").Flush();

        return false;
    }

    return true;
}

void Signable::update_version(const VersionNumber version) noexcept
{
    signatures_.clear();
    const_cast<VersionNumber&>(version_) = version;
}

bool Signable::Validate() const
{
    Lock lock(lock_);

    return validate(lock);
}

bool Signable::verify_write_lock(const Lock& lock) const
{
    if (lock.mutex() != &lock_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock not owned.").Flush();

        return false;
    }

    return true;
}

bool Signable::verify_signature(const Lock& lock, const proto::Signature&) const
{
    OT_ASSERT(verify_write_lock(lock));

    if (!nym_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing nym.").Flush();

        return false;
    }

    return true;
}

VersionNumber Signable::Version() const { return version_; }
}  // namespace opentxs::contract::implementation
