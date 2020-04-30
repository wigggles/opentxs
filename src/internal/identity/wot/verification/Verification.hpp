// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/identity/wot/verification/Group.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/identity/wot/verification/Nym.hpp"
#include "opentxs/identity/wot/verification/Set.hpp"

namespace opentxs::identity::wot::verification::internal
{
struct Group : virtual public verification::Group {
    virtual const api::internal::Core& API() const noexcept = 0;
    virtual bool External() const noexcept = 0;
    virtual const identifier::Nym& NymID() const noexcept = 0;

    virtual void Register(
        const Identifier& id,
        const identifier::Nym& nym) noexcept = 0;
    virtual void Unregister(const Identifier& id) noexcept = 0;
    virtual bool UpgradeNymVersion(const VersionNumber nymVersion) noexcept = 0;

    ~Group() override = default;
};
struct Item : virtual public verification::Item {
    ~Item() override = default;
};
struct Nym : virtual public verification::Nym {
    virtual const api::internal::Core& API() const noexcept = 0;
    virtual const identifier::Nym& NymID() const noexcept = 0;

    using verification::Nym::AddItem;
    virtual bool AddItem(const Item::SerializedType item) noexcept = 0;
    virtual bool UpgradeItemVersion(
        const VersionNumber itemVersion,
        VersionNumber& nymVersion) noexcept = 0;

    ~Nym() override = default;
};
struct Set : virtual public verification::Set {
    virtual const api::internal::Core& API() const noexcept = 0;
    virtual const identifier::Nym& NymID() const noexcept = 0;

    virtual void Register(
        const Identifier& id,
        const bool external) noexcept = 0;
    virtual void Unregister(const Identifier& id) noexcept = 0;
    virtual bool UpgradeGroupVersion(
        const VersionNumber groupVersion) noexcept = 0;

    ~Set() override = default;
};
}  // namespace opentxs::identity::wot::verification::internal
