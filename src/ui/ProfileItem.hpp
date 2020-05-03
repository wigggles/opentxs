// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/ProfileItem.cpp"

#pragma once

#include <memory>
#include <string>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/ui/ProfileItem.hpp"
#include "ui/Row.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace client
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class ProfileItem;
}  // namespace ui

class Factory;
}  // namespace opentxs

template class opentxs::SharedPimpl<opentxs::ui::ProfileItem>;

namespace opentxs::ui::implementation
{
using ProfileItemRow =
    Row<ProfileSubsectionRowInternal,
        ProfileSubsectionInternalInterface,
        ProfileSubsectionRowID>;

class ProfileItem final : public ProfileItemRow
{
public:
    std::string ClaimID() const noexcept final
    {
        sLock lock(shared_lock_);

        return row_id_->str();
    }
    bool Delete() const noexcept final;
    bool IsActive() const noexcept final
    {
        sLock lock(shared_lock_);

        return item_->isActive();
    }
    bool IsPrimary() const noexcept final
    {
        sLock lock(shared_lock_);

        return item_->isPrimary();
    }
    bool SetActive(const bool& active) const noexcept final;
    bool SetPrimary(const bool& primary) const noexcept final;
    bool SetValue(const std::string& value) const noexcept final;
    std::string Value() const noexcept final
    {
        sLock lock(shared_lock_);

        return item_->Value();
    }

    void reindex(
        const ProfileSubsectionSortKey& key,
        const CustomData& custom) noexcept final;

    ProfileItem(
        const ProfileSubsectionInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ProfileSubsectionRowID& rowID,
        const ProfileSubsectionSortKey& sortKey,
        const CustomData& custom) noexcept;
    ~ProfileItem() = default;

private:
    friend opentxs::Factory;

    std::unique_ptr<opentxs::ContactItem> item_;

    bool add_claim(const Claim& claim) const noexcept;
    Claim as_claim() const noexcept;

    ProfileItem() = delete;
    ProfileItem(const ProfileItem&) = delete;
    ProfileItem(ProfileItem&&) = delete;
    ProfileItem& operator=(const ProfileItem&) = delete;
    ProfileItem& operator=(ProfileItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
