// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <list>
#include <mutex>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/Signable.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace proto
{
class Signature;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::contract::implementation
{
class Signable : virtual public opentxs::contract::Signable
{
public:
    std::string Alias() const override;
    OTIdentifier ID() const override;
    Nym_p Nym() const override;
    const std::string& Terms() const override;
    bool Validate() const override;
    VersionNumber Version() const override;

    void SetAlias(const std::string& alias) override;

    ~Signable() override = default;

protected:
    using Signatures = std::list<Signature>;

    const api::internal::Core& api_;
    mutable std::mutex lock_;
    const Nym_p nym_;
    const VersionNumber version_;
    const std::string conditions_;
    const OTIdentifier id_;
    Signatures signatures_;
    std::string alias_;

    bool CheckID(const Lock& lock) const;
    virtual OTIdentifier id(const Lock& lock) const;
    virtual bool validate(const Lock& lock) const = 0;
    virtual bool verify_signature(
        const Lock& lock,
        const proto::Signature& signature) const;
    bool verify_write_lock(const Lock& lock) const;

    virtual void first_time_init(const Lock& lock) noexcept(false);
    virtual void init_serialized(const Lock& lock) noexcept(false);
    virtual bool update_signature(
        const Lock& lock,
        const PasswordPrompt& reason);
    void update_version(const VersionNumber version) noexcept;

    virtual OTIdentifier GetID(const Lock& lock) const = 0;

    Signable(
        const api::internal::Core& api,
        const Nym_p& nym,
        const VersionNumber version,
        const std::string& conditions,
        const std::string& alias) noexcept;
    Signable(
        const api::internal::Core& api,
        const Nym_p& nym,
        const VersionNumber version,
        const std::string& conditions,
        const std::string& alias,
        OTIdentifier&& id,
        Signatures&& signatures) noexcept;
    Signable(const Signable&) noexcept;
    Signable(Signable&&) = delete;
    Signable& operator=(const Signable&) = delete;
    Signable& operator=(Signable&&) = delete;
};
}  // namespace opentxs::contract::implementation
