// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "ui/ContactSubsection.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <type_traits>

#include "opentxs/Pimpl.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/protobuf/verify/VerifyContacts.hpp"
#include "ui/Combined.hpp"
#include "ui/Widget.hpp"

//#define OT_METHOD "opentxs::ui::implementation::ContactSubsection::"

namespace opentxs::factory
{
auto ContactSubsectionWidget(
    const ui::implementation::ContactSectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const ui::implementation::ContactSectionRowID& rowID,
    const ui::implementation::ContactSectionSortKey& key,
    ui::implementation::CustomData& custom
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::shared_ptr<ui::implementation::ContactSectionRowInternal>
{
    using ReturnType = ui::implementation::ContactSubsection;

    return std::make_shared<ReturnType>(
        parent,
        api,
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
ContactSubsection::ContactSubsection(
    const ContactSectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const ContactSectionRowID& rowID,
    const ContactSectionSortKey& key,
    CustomData& custom
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    : Combined(
          api,
          Identifier::Factory(parent.ContactID()),
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
    startup_.reset(new std::thread(
        &ContactSubsection::startup,
        this,
        extract_custom<opentxs::ContactGroup>(custom)));

    OT_ASSERT(startup_)
}

auto ContactSubsection::construct_row(
    const ContactSubsectionRowID& id,
    const ContactSubsectionSortKey& index,
    CustomData& custom) const noexcept -> void*
{
    OT_ASSERT(1 == custom.size())

    names_.emplace(id, index);
    const auto [it, added] = items_[index].emplace(
        id, factory::ContactItemWidget(*this, api_, id, index, custom));

    return it->second.get();
}

auto ContactSubsection::Name(const std::string& lang) const noexcept
    -> std::string
{
    return proto::TranslateItemType(row_id_.second, lang);
}

auto ContactSubsection::process_group(
    const opentxs::ContactGroup& group) noexcept
    -> std::set<ContactSubsectionRowID>
{
    OT_ASSERT(row_id_.second == group.Type())

    std::set<ContactSubsectionRowID> active{};

    for (const auto& [id, claim] : group) {
        OT_ASSERT(claim)

        CustomData custom{new opentxs::ContactItem(*claim)};
        add_item(id, sort_key(id), custom);
        active.emplace(id);
    }

    return active;
}

void ContactSubsection::reindex(
    const ContactSectionSortKey&,
    CustomData& custom) noexcept
{
    delete_inactive(
        process_group(extract_custom<opentxs::ContactGroup>(custom)));
}

auto ContactSubsection::sort_key(const ContactSubsectionRowID) const noexcept
    -> int
{
    return static_cast<int>(items_.size());
}

void ContactSubsection::startup(const opentxs::ContactGroup group) noexcept
{
    process_group(group);
    finish_startup();
}
}  // namespace opentxs::ui::implementation
