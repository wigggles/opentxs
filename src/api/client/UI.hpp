// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/client/UI.cpp"

#pragma once

#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountSummary.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/Contact.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/List.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "opentxs/ui/PayableList.hpp"
#include "opentxs/ui/Profile.hpp"

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

namespace ui
{
namespace implementation
{
class AccountActivity;
class AccountList;
class AccountSummary;
class ActivitySummary;
class ActivityThread;
class Contact;
class ContactList;
class MessagableList;
class PayableList;
class Profile;
}  // namespace implementation
}  // namespace ui

class Factory;
class Flag;
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
class UI final : virtual public opentxs::api::client::UI, Lockable
{
public:
    const ui::AccountActivity& AccountActivity(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept final;
    const ui::AccountList& AccountList(const identifier::Nym& nym) const
        noexcept final;
    const ui::AccountSummary& AccountSummary(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const noexcept final;
    const ui::ActivitySummary& ActivitySummary(
        const identifier::Nym& nymID) const noexcept final;
    const ui::ActivityThread& ActivityThread(
        const identifier::Nym& nymID,
        const Identifier& threadID) const noexcept final;
    const ui::Contact& Contact(const Identifier& contactID) const
        noexcept final;
    const ui::ContactList& ContactList(const identifier::Nym& nymID) const
        noexcept final;
    const ui::MessagableList& MessagableList(const identifier::Nym& nymID) const
        noexcept final;
    const ui::PayableList& PayableList(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const noexcept final;
    const ui::Profile& Profile(const identifier::Nym& nymID) const
        noexcept final;

#if OT_QT
    ui::AccountActivityQt* AccountActivityQt(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept final;
    ui::AccountListQt* AccountListQt(const identifier::Nym& nym) const
        noexcept final;
    ui::AccountSummaryQt* AccountSummaryQt(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const noexcept final;
    ui::ActivitySummaryQt* ActivitySummaryQt(const identifier::Nym& nymID) const
        noexcept final;
    ui::ActivityThreadQt* ActivityThreadQt(
        const identifier::Nym& nymID,
        const Identifier& threadID) const noexcept final;
    QAbstractItemModel* BlankModel(const std::size_t columns) const
        noexcept final
    {
        return blank_.get(columns);
    }
    ui::ContactQt* ContactQt(const Identifier& contactID) const noexcept final;
    ui::ContactListQt* ContactListQt(const identifier::Nym& nymID) const
        noexcept final;
    ui::MessagableListQt* MessagableListQt(const identifier::Nym& nymID) const
        noexcept final;
    ui::PayableListQt* PayableListQt(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const noexcept final;
    ui::ProfileQt* ProfileQt(const identifier::Nym& nymID) const noexcept final;
#endif  // OT_QT

    ~UI() final = default;

private:
    friend opentxs::Factory;

    /** NymID, AccountID */
    using AccountActivityKey = std::pair<OTNymID, OTIdentifier>;
    using AccountListKey = OTNymID;
    /** NymID, currency*/
    using AccountSummaryKey = std::pair<OTNymID, proto::ContactItemType>;
    using ActivitySummaryKey = OTNymID;
    using ActivityThreadKey = std::pair<OTNymID, OTIdentifier>;
    using ContactKey = OTIdentifier;
    using ContactListKey = OTNymID;
    using MessagableListKey = OTNymID;
    /** NymID, currency*/
    using PayableListKey = std::pair<OTNymID, proto::ContactItemType>;
    using ProfileKey = OTNymID;

    using AccountActivityValue =
        std::unique_ptr<ui::implementation::AccountActivity>;
    using AccountListValue = std::unique_ptr<ui::implementation::AccountList>;
    using AccountSummaryValue =
        std::unique_ptr<ui::implementation::AccountSummary>;
    using ActivitySummaryValue =
        std::unique_ptr<ui::implementation::ActivitySummary>;
    using ActivityThreadValue =
        std::unique_ptr<ui::implementation::ActivityThread>;
    using ContactValue = std::unique_ptr<ui::implementation::Contact>;
    using ContactListValue = std::unique_ptr<ui::implementation::ContactList>;
    using MessagableListValue =
        std::unique_ptr<ui::implementation::MessagableList>;
    using PayableListValue = std::unique_ptr<ui::implementation::PayableList>;
    using ProfileValue = std::unique_ptr<ui::implementation::Profile>;
    using AccountActivityMap =
        std::map<AccountActivityKey, AccountActivityValue>;
    using AccountListMap = std::map<AccountListKey, AccountListValue>;
    using AccountSummaryMap = std::map<AccountSummaryKey, AccountSummaryValue>;
    using ActivitySummaryMap =
        std::map<ActivitySummaryKey, ActivitySummaryValue>;
    using ActivityThreadMap = std::map<ActivityThreadKey, ActivityThreadValue>;
    using ContactMap = std::map<ContactKey, ContactValue>;
    using ContactListMap = std::map<ContactListKey, ContactListValue>;
    using MessagableListMap = std::map<MessagableListKey, MessagableListValue>;
    using PayableListMap = std::map<PayableListKey, PayableListValue>;
    using ProfileMap = std::map<ProfileKey, ProfileValue>;

#if OT_QT
    using AccountActivityQtValue = std::unique_ptr<ui::AccountActivityQt>;
    using AccountListQtValue = std::unique_ptr<ui::AccountListQt>;
    using AccountSummaryQtValue = std::unique_ptr<ui::AccountSummaryQt>;
    using ActivitySummaryQtValue = std::unique_ptr<ui::ActivitySummaryQt>;
    using ActivityThreadQtValue = std::unique_ptr<ui::ActivityThreadQt>;
    using ContactQtValue = std::unique_ptr<ui::ContactQt>;
    using ContactListQtValue = std::unique_ptr<ui::ContactListQt>;
    using MessagableListQtValue = std::unique_ptr<ui::MessagableListQt>;
    using PayableListQtValue = std::unique_ptr<ui::PayableListQt>;
    using ProfileQtValue = std::unique_ptr<ui::ProfileQt>;
    using AccountActivityQtMap =
        std::map<AccountActivityKey, AccountActivityQtValue>;
    using AccountListQtMap = std::map<AccountListKey, AccountListQtValue>;
    using AccountSummaryQtMap =
        std::map<AccountSummaryKey, AccountSummaryQtValue>;
    using ActivitySummaryQtMap =
        std::map<ActivitySummaryKey, ActivitySummaryQtValue>;
    using ActivityThreadQtMap =
        std::map<ActivityThreadKey, ActivityThreadQtValue>;
    using ContactQtMap = std::map<ContactKey, ContactQtValue>;
    using ContactListQtMap = std::map<ContactListKey, ContactListQtValue>;
    using MessagableListQtMap =
        std::map<MessagableListKey, MessagableListQtValue>;
    using PayableListQtMap = std::map<PayableListKey, PayableListQtValue>;
    using ProfileQtMap = std::map<ProfileKey, ProfileQtValue>;

    struct Blank {
        ui::BlankModel* get(const std::size_t columns) noexcept;

    private:
        std::mutex lock_{};
        std::map<std::size_t, ui::BlankModel> map_{};
    };
#endif  // OT_QT

    const api::client::internal::Manager& api_;
    const Flag& running_;
#if OT_QT
    const bool enable_qt_;
#endif  // OT_QT
    mutable AccountActivityMap accounts_;
    mutable AccountListMap account_lists_;
    mutable AccountSummaryMap account_summaries_;
    mutable ActivitySummaryMap activity_summaries_;
    mutable ContactMap contacts_;
    mutable ContactListMap contact_lists_;
    mutable MessagableListMap messagable_lists_;
    mutable PayableListMap payable_lists_;
    mutable ActivityThreadMap activity_threads_;
    mutable ProfileMap profiles_;
#if OT_QT
    mutable Blank blank_;
    mutable AccountActivityQtMap accounts_qt_;
    mutable AccountListQtMap account_lists_qt_;
    mutable AccountSummaryQtMap account_summaries_qt_;
    mutable ActivitySummaryQtMap activity_summaries_qt_;
    mutable ContactQtMap contacts_qt_;
    mutable ContactListQtMap contact_lists_qt_;
    mutable MessagableListQtMap messagable_lists_qt_;
    mutable PayableListQtMap payable_lists_qt_;
    mutable ActivityThreadQtMap activity_threads_qt_;
    mutable ProfileQtMap profiles_qt_;
#endif  // OT_QT
    OTZMQPublishSocket widget_update_publisher_;

    AccountActivityMap::mapped_type& account_activity(
        const Lock& lock,
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept;
    AccountListMap::mapped_type& account_list(
        const Lock& lock,
        const identifier::Nym& nymID) const noexcept;
    AccountSummaryMap::mapped_type& account_summary(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const noexcept;
    ActivitySummaryMap::mapped_type& activity_summary(
        const Lock& lock,
        const identifier::Nym& nymID) const noexcept;
    ActivityThreadMap::mapped_type& activity_thread(
        const Lock& lock,
        const identifier::Nym& nymID,
        const Identifier& threadID) const noexcept;
    ContactMap::mapped_type& contact(
        const Lock& lock,
        const Identifier& contactID) const noexcept;
    ContactListMap::mapped_type& contact_list(
        const Lock& lock,
        const identifier::Nym& nymID) const noexcept;
    MessagableListMap::mapped_type& messagable_list(
        const Lock& lock,
        const identifier::Nym& nymID) const noexcept;
    PayableListMap::mapped_type& payable_list(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const noexcept;
    ProfileMap::mapped_type& profile(
        const Lock& lock,
        const identifier::Nym& nymID) const noexcept;

    UI(const api::client::internal::Manager& api,
       const Flag& running
#if OT_QT
       ,
       const bool qt
#endif  // OT_QT
       )
    noexcept;
    UI() = delete;
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};
}  // namespace opentxs::api::client::implementation
