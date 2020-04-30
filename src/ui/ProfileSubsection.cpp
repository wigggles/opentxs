// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "ui/ProfileSubsection.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <type_traits>

#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "ui/Combined.hpp"
#include "ui/Widget.hpp"

//#define OT_METHOD "opentxs::ui::implementation::ProfileSubsection::"

namespace opentxs::factory
{
auto ProfileSubsectionWidget(
    const ui::implementation::ProfileSectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::ProfileSectionRowID& rowID,
    const ui::implementation::ProfileSectionSortKey& key,
    const ui::implementation::CustomData& custom
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::shared_ptr<ui::implementation::ProfileSectionRowInternal>
{
    using ReturnType = ui::implementation::ProfileSubsection;

    return std::make_shared<ReturnType>(
        parent,
        api,
        publisher,
        rowID,
        key,
        custom
#if OT_QT
        ,
        qt
#endif
    );
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ProfileSubsection::ProfileSubsection(
    const ProfileSectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ProfileSectionRowID& rowID,
    const ProfileSectionSortKey& key,
    const CustomData& custom
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    : Combined(
          api,
          publisher,
          parent.NymID(),
          parent.WidgetID(),
          parent,
          rowID,
          key
#if OT_QT
          ,
          qt
#endif
      )
{
    init();
    startup_.reset(new std::thread(&ProfileSubsection::startup, this, custom));

    OT_ASSERT(startup_)
}

bool ProfileSubsection::AddItem(
    const std::string& value,
    const bool primary,
    const bool active) const noexcept
{
    return parent_.AddClaim(row_id_.second, value, primary, active);
}

void* ProfileSubsection::construct_row(
    const ProfileSubsectionRowID& id,
    const ProfileSubsectionSortKey& index,
    const CustomData& custom) const noexcept
{
    OT_ASSERT(1 == custom.size())

    names_.emplace(id, index);
    const auto [it, added] = items_[index].emplace(
        id,
        factory::ProfileItemWidget(*this, api_, publisher_, id, index, custom));

    return it->second.get();
}

bool ProfileSubsection::Delete(const std::string& claimID) const noexcept
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.Delete();
}

std::string ProfileSubsection::Name(const std::string& lang) const noexcept
{
    return proto::TranslateItemType(row_id_.second, lang);
}

std::set<ProfileSubsectionRowID> ProfileSubsection::process_group(
    const opentxs::ContactGroup& group) noexcept
{
    OT_ASSERT(row_id_.second == group.Type())

    std::set<ProfileSubsectionRowID> active{};

    for (const auto& [id, claim] : group) {
        OT_ASSERT(claim)

        CustomData custom{new opentxs::ContactItem(*claim)};
        add_item(id, sort_key(id), custom);
        active.emplace(id);
    }

    return active;
}

void ProfileSubsection::reindex(
    const ProfileSectionSortKey&,
    const CustomData& custom) noexcept
{
    delete_inactive(
        process_group(extract_custom<opentxs::ContactGroup>(custom)));
}

bool ProfileSubsection::SetActive(const std::string& claimID, const bool active)
    const noexcept
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.SetActive(active);
}

bool ProfileSubsection::SetPrimary(
    const std::string& claimID,
    const bool primary) const noexcept
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.SetPrimary(primary);
}

bool ProfileSubsection::SetValue(
    const std::string& claimID,
    const std::string& value) const noexcept
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.SetValue(value);
}

int ProfileSubsection::sort_key(const ProfileSubsectionRowID) const noexcept
{
    return static_cast<int>(items_.size());
}

void ProfileSubsection::startup(const CustomData& custom) noexcept
{
    process_group(extract_custom<opentxs::ContactGroup>(custom));
    finish_startup();
}
}  // namespace opentxs::ui::implementation
