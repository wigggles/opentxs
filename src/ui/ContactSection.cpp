// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "ui/ContactSection.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>

#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "ui/Combined.hpp"
#include "ui/Widget.hpp"

//#define OT_METHOD "opentxs::ui::implementation::ContactSection::"

namespace opentxs::factory
{
auto ContactSectionWidget(
    const ui::implementation::ContactInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::ContactRowID& rowID,
    const ui::implementation::ContactSortKey& key,
    const ui::implementation::CustomData& custom
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::shared_ptr<ui::implementation::ContactRowInternal>
{
    using ReturnType = ui::implementation::ContactSection;

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
const std::map<proto::ContactSectionName, std::set<proto::ContactItemType>>
    ContactSection::allowed_types_{
        {proto::CONTACTSECTION_COMMUNICATION,
         {
             proto::CITEMTYPE_PHONE,
             proto::CITEMTYPE_EMAIL,
             proto::CITEMTYPE_SKYPE,
             proto::CITEMTYPE_WIRE,
             proto::CITEMTYPE_QQ,
             proto::CITEMTYPE_BITMESSAGE,
             proto::CITEMTYPE_WHATSAPP,
             proto::CITEMTYPE_TELEGRAM,
             proto::CITEMTYPE_KIK,
             proto::CITEMTYPE_BBM,
             proto::CITEMTYPE_WECHAT,
             proto::CITEMTYPE_KAKAOTALK,
         }},
        {proto::CONTACTSECTION_PROFILE,
         {
             proto::CITEMTYPE_FACEBOOK,  proto::CITEMTYPE_GOOGLE,
             proto::CITEMTYPE_LINKEDIN,  proto::CITEMTYPE_VK,
             proto::CITEMTYPE_ABOUTME,   proto::CITEMTYPE_ONENAME,
             proto::CITEMTYPE_TWITTER,   proto::CITEMTYPE_MEDIUM,
             proto::CITEMTYPE_TUMBLR,    proto::CITEMTYPE_YAHOO,
             proto::CITEMTYPE_MYSPACE,   proto::CITEMTYPE_MEETUP,
             proto::CITEMTYPE_REDDIT,    proto::CITEMTYPE_HACKERNEWS,
             proto::CITEMTYPE_WIKIPEDIA, proto::CITEMTYPE_ANGELLIST,
             proto::CITEMTYPE_GITHUB,    proto::CITEMTYPE_BITBUCKET,
             proto::CITEMTYPE_YOUTUBE,   proto::CITEMTYPE_VIMEO,
             proto::CITEMTYPE_TWITCH,    proto::CITEMTYPE_SNAPCHAT,
         }},
    };

const std::map<proto::ContactSectionName, std::map<proto::ContactItemType, int>>
    ContactSection::sort_keys_{
        {proto::CONTACTSECTION_COMMUNICATION,
         {
             {proto::CITEMTYPE_PHONE, 0},
             {proto::CITEMTYPE_EMAIL, 1},
             {proto::CITEMTYPE_SKYPE, 2},
             {proto::CITEMTYPE_TELEGRAM, 3},
             {proto::CITEMTYPE_WIRE, 4},
             {proto::CITEMTYPE_WECHAT, 5},
             {proto::CITEMTYPE_QQ, 6},
             {proto::CITEMTYPE_KIK, 7},
             {proto::CITEMTYPE_KAKAOTALK, 8},
             {proto::CITEMTYPE_BBM, 9},
             {proto::CITEMTYPE_WHATSAPP, 10},
             {proto::CITEMTYPE_BITMESSAGE, 11},
         }},
        {proto::CONTACTSECTION_PROFILE,
         {
             {proto::CITEMTYPE_FACEBOOK, 0},
             {proto::CITEMTYPE_TWITTER, 1},
             {proto::CITEMTYPE_REDDIT, 2},
             {proto::CITEMTYPE_GOOGLE, 3},
             {proto::CITEMTYPE_SNAPCHAT, 4},
             {proto::CITEMTYPE_YOUTUBE, 5},
             {proto::CITEMTYPE_TWITCH, 6},
             {proto::CITEMTYPE_GITHUB, 7},
             {proto::CITEMTYPE_LINKEDIN, 8},
             {proto::CITEMTYPE_MEDIUM, 9},
             {proto::CITEMTYPE_TUMBLR, 10},
             {proto::CITEMTYPE_YAHOO, 11},
             {proto::CITEMTYPE_MYSPACE, 12},
             {proto::CITEMTYPE_VK, 13},
             {proto::CITEMTYPE_MEETUP, 14},
             {proto::CITEMTYPE_VIMEO, 15},
             {proto::CITEMTYPE_ANGELLIST, 16},
             {proto::CITEMTYPE_ONENAME, 17},
             {proto::CITEMTYPE_ABOUTME, 18},
             {proto::CITEMTYPE_BITBUCKET, 19},
             {proto::CITEMTYPE_WIKIPEDIA, 20},
             {proto::CITEMTYPE_HACKERNEWS, 21},
         }},
    };

ContactSection::ContactSection(
    const ContactInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ContactRowID& rowID,
    const ContactSortKey& key,
    const CustomData& custom
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    : Combined(
          api,
          publisher,
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
    startup_.reset(new std::thread(&ContactSection::startup, this, custom));

    OT_ASSERT(startup_)
}

auto ContactSection::check_type(const ContactSectionRowID type) noexcept -> bool
{
    try {
        return 1 == allowed_types_.at(type.first).count(type.second);
    } catch (const std::out_of_range&) {
    }

    return false;
}

auto ContactSection::construct_row(
    const ContactSectionRowID& id,
    const ContactSectionSortKey& index,
    const CustomData& custom) const noexcept -> void*
{
    OT_ASSERT(1 == custom.size())

    names_.emplace(id, index);
    const auto [it, added] = items_[index].emplace(
        id,
        factory::ContactSubsectionWidget(
            *this,
            api_,
            publisher_,
            id,
            index,
            custom
#if OT_QT
            ,
            enable_qt_
#endif
            ));

    return it->second.get();
}

auto ContactSection::process_section(
    const opentxs::ContactSection& section) noexcept
    -> std::set<ContactSectionRowID>
{
    OT_ASSERT(row_id_ == section.Type())

    std::set<ContactSectionRowID> active{};

    for (const auto& [type, group] : section) {
        OT_ASSERT(group)

        const ContactSectionRowID key{row_id_, type};

        if (check_type(key)) {
            CustomData custom{new opentxs::ContactGroup(*group)};
            add_item(key, sort_key(key), custom);
            active.emplace(key);
        }
    }

    return active;
}

void ContactSection::reindex(
    const implementation::ContactSortKey&,
    const implementation::CustomData& custom) noexcept
{
    delete_inactive(
        process_section(extract_custom<opentxs::ContactSection>(custom)));
}

auto ContactSection::sort_key(const ContactSectionRowID type) noexcept -> int
{
    return sort_keys_.at(type.first).at(type.second);
}

void ContactSection::startup(const CustomData custom) noexcept
{
    process_section(extract_custom<opentxs::ContactSection>(custom));
    finish_startup();
}
}  // namespace opentxs::ui::implementation
