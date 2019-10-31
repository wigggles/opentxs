// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/identifier/Nym.hpp"

#include "internal/identity/wot/verification/Verification.hpp"

#include <memory>
#include <stdexcept>

#include "Set.hpp"

#define OT_METHOD "opentxs::identity::wot::verification::implementation::Set::"

namespace opentxs
{
identity::wot::verification::internal::Set* Factory::VerificationSet(
    const api::internal::Core& api,
    const identifier::Nym& nym,
    const VersionNumber version)
{
    using ReturnType =
        opentxs::identity::wot::verification::implementation::Set;

    try {

        return new ReturnType(api, nym, version);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            "Failed to construct verification nym: ")(e.what())
            .Flush();

        return nullptr;
    }
}

identity::wot::verification::internal::Set* Factory::VerificationSet(
    const api::internal::Core& api,
    const identifier::Nym& nym,
    const proto::VerificationSet& serialized)
{
    using ReturnType =
        opentxs::identity::wot::verification::implementation::Set;

    try {

        return new ReturnType(api, nym, serialized);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            "Failed to construct verification nym: ")(e.what())
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs

namespace opentxs::identity::wot::verification
{
const VersionNumber Set::DefaultVersion{1};
}

namespace opentxs::identity::wot::verification::implementation
{
Set::Set(
    const api::internal::Core& api,
    const identifier::Nym& nym,
    const VersionNumber version) noexcept(false)
    : api_(api)
    , version_(version)
    , nym_id_(nym)
    , internal_(instantiate(*this, ChildType::default_instance(), false))
    , external_(instantiate(*this, ChildType::default_instance(), true))
    , map_()
{
    if (false == bool(internal_)) {
        throw std::runtime_error("Failed to instantiate internal group");
    }

    if (false == bool(external_)) {
        throw std::runtime_error("Failed to instantiate external group");
    }
}

Set::Set(
    const api::internal::Core& api,
    const identifier::Nym& nym,
    const SerializedType& in) noexcept(false)
    : api_(api)
    , version_(in.version())
    , nym_id_(nym)
    , internal_(instantiate(*this, in.internal(), false))
    , external_(instantiate(*this, in.external(), true))
    , map_()
{
    if (false == bool(internal_)) {
        throw std::runtime_error("Failed to instantiate internal group");
    }

    if (false == bool(external_)) {
        throw std::runtime_error("Failed to instantiate external group");
    }
}

Set::operator SerializedType() const noexcept
{
    auto output = SerializedType{};
    output.set_version(version_);
    output.mutable_internal()->CopyFrom(*internal_);
    output.mutable_external()->CopyFrom(*external_);

    return output;
}

auto Set::AddItem(
    const identifier::Nym& claimOwner,
    const Identifier& claim,
    const identity::Nym& signer,
    const PasswordPrompt& reason,
    const Item::Type value,
    const Time start,
    const Time end,
    const VersionNumber version) noexcept -> bool
{
    OT_ASSERT(internal_);

    return internal_->AddItem(
        claimOwner, claim, signer, reason, value, start, end, version);
}

auto Set::AddItem(
    const identifier::Nym& verifier,
    const Item::SerializedType verification) noexcept -> bool
{
    OT_ASSERT(external_);

    return external_->AddItem(verifier, verification);
}

auto Set::DeleteItem(const Identifier& item) noexcept -> bool
{
    auto it = map_.find(item);

    if (map_.end() == it) { return false; }

    auto* pGroup = (it->second ? external_ : internal_).get();

    OT_ASSERT(pGroup);

    auto& group = *pGroup;

    return group.DeleteItem(item);
}

auto Set::instantiate(
    internal::Set& parent,
    const ChildType& in,
    bool external) noexcept -> GroupPointer
{
    auto output = GroupPointer{};

    if (opentxs::operator==(ChildType::default_instance(), in)) {
        output.reset(Factory::VerificationGroup(
            parent, Group::DefaultVersion, external));
    } else {
        output.reset(Factory::VerificationGroup(parent, in, external));
    }

    return output;
}

auto Set::Register(const Identifier& id, const bool external) noexcept -> void
{
    auto it = map_.find(id);

    if (map_.end() == it) {
        map_.emplace(id, external);
    } else {
        it->second = external;
    }
}

auto Set::Unregister(const Identifier& id) noexcept -> void { map_.erase(id); }

auto Set::UpgradeGroupVersion(const VersionNumber groupVersion) noexcept -> bool
{
    auto nymVersion{version_};

    try {
        while (true) {
            const auto [min, max] =
                proto::VerificationSetAllowedGroup().at(nymVersion);

            if (groupVersion < min) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Version ")(groupVersion)(
                    " too old")
                    .Flush();

                return false;
            }

            if (groupVersion > max) {
                ++nymVersion;
            } else {

                return true;
            }
        }
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No support for version ")(
            groupVersion)(" groups")
            .Flush();

        return false;
    }
}
}  // namespace opentxs::identity::wot::verification::implementation
