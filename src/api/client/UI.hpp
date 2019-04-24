// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class UI : virtual public opentxs::api::client::UI, Lockable
{
public:
    const ui::AccountActivity& AccountActivity(
        const identifier::Nym& nymID,
        const Identifier& accountID) const override;
    const ui::AccountList& AccountList(
        const identifier::Nym& nym) const override;
    const ui::AccountSummary& AccountSummary(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const override;
    const ui::ActivitySummary& ActivitySummary(
        const identifier::Nym& nymID) const override;
    const ui::ActivityThread& ActivityThread(
        const identifier::Nym& nymID,
        const Identifier& threadID) const override;
    const ui::Contact& Contact(const Identifier& contactID) const override;
    const ui::ContactList& ContactList(
        const identifier::Nym& nymID) const override;
    const ui::MessagableList& MessagableList(
        const identifier::Nym& nymID) const override;
    const ui::PayableList& PayableList(
        const identifier::Nym& nymID,
        proto::ContactItemType currency) const override;
    const ui::Profile& Profile(const identifier::Nym& nymID) const override;

#if OT_QT
    ui::AccountActivityQt* AccountActivityQt(
        const identifier::Nym& nymID,
        const Identifier& accountID) const override;
    ui::AccountListQt* AccountListQt(const identifier::Nym& nym) const override;
    ui::ActivitySummaryQt* ActivitySummaryQt(
        const identifier::Nym& nymID) const override;
    ui::ActivityThreadQt* ActivityThreadQt(
        const identifier::Nym& nymID,
        const Identifier& threadID) const override;
    ui::ContactQt* ContactQt(const Identifier& contactID) const override;
    ui::ContactListQt* ContactListQt(
        const identifier::Nym& nymID) const override;
    ui::ProfileQt* ProfileQt(const identifier::Nym& nymID) const override;
#endif

    ~UI() override = default;

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

#if OT_QT
    using AccountActivityValue = ui::AccountActivityQt;
    using AccountListValue = ui::AccountListQt;
    using AccountSummaryValue = std::unique_ptr<ui::AccountSummary>;
    using ActivitySummaryValue = ui::ActivitySummaryQt;
    using ActivityThreadValue = ui::ActivityThreadQt;
    using ContactValue = ui::ContactQt;
    using ContactListValue = ui::ContactListQt;
    using MessagableListValue = std::unique_ptr<ui::MessagableList>;
    using PayableListValue = std::unique_ptr<ui::PayableList>;
    using ProfileValue = ui::ProfileQt;
#else
    using AccountActivityValue = std::unique_ptr<ui::AccountActivity>;
    using AccountListValue = std::unique_ptr<ui::AccountList>;
    using AccountSummaryValue = std::unique_ptr<ui::AccountSummary>;
    using ActivitySummaryValue = std::unique_ptr<ui::ActivitySummary>;
    using ActivityThreadValue = std::unique_ptr<ui::ActivityThread>;
    using ContactValue = std::unique_ptr<ui::Contact>;
    using ContactListValue = std::unique_ptr<ui::ContactList>;
    using MessagableListValue = std::unique_ptr<ui::MessagableList>;
    using PayableListValue = std::unique_ptr<ui::PayableList>;
    using ProfileValue = std::unique_ptr<ui::Profile>;
#endif

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

    const api::client::Manager& api_;
    const Flag& running_;
#if OT_QT
    const bool enable_qt_;
#endif
    mutable AccountActivityMap accounts_{};
    mutable AccountListMap account_lists_{};
    mutable AccountSummaryMap accounts_summaries_{};
    mutable ActivitySummaryMap activity_summaries_{};
    mutable ContactMap contacts_{};
    mutable ContactListMap contact_lists_{};
    mutable MessagableListMap messagable_lists_{};
    mutable PayableListMap payable_lists_{};
    mutable ActivityThreadMap activity_threads_{};
    mutable ProfileMap profiles_{};
    OTZMQPublishSocket widget_update_publisher_;

    AccountActivityMap::mapped_type& account_activity(
        const identifier::Nym& nymID,
        const Identifier& accountID) const;
    AccountListMap::mapped_type& account_list(
        const identifier::Nym& nymID) const;
    ActivitySummaryMap::mapped_type& activity_summary(
        const identifier::Nym& nymID) const;
    ActivityThreadMap::mapped_type& activity_thread(
        const identifier::Nym& nymID,
        const Identifier& threadID) const;
    ContactMap::mapped_type& contact(const Identifier& contactID) const;
    ContactListMap::mapped_type& contact_list(
        const identifier::Nym& nymID) const;
    ProfileMap::mapped_type& profile(const identifier::Nym& nymID) const;

    UI(const api::client::Manager& api,
       const Flag& running
#if OT_QT
       ,
       const bool qt
#endif
    );
    UI() = delete;
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};
}  // namespace opentxs::api::client::implementation
