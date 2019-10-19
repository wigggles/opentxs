// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::client::implementation
{
class UI final : virtual public opentxs::api::client::UI, Lockable
{
public:
    const ui::AccountActivity& AccountActivity(
        const identifier::Nym& nymID,
        const Identifier& accountID) const final;
    const ui::AccountList& AccountList(const identifier::Nym& nym) const final;
    const ui::AccountSummary& AccountSummary(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const final;
    const ui::ActivitySummary& ActivitySummary(
        const identifier::Nym& nymID) const final;
    const ui::ActivityThread& ActivityThread(
        const identifier::Nym& nymID,
        const Identifier& threadID) const final;
    const ui::Contact& Contact(const Identifier& contactID) const final;
    const ui::ContactList& ContactList(
        const identifier::Nym& nymID) const final;
    const ui::MessagableList& MessagableList(
        const identifier::Nym& nymID) const final;
    const ui::PayableList& PayableList(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const final;
    const ui::Profile& Profile(const identifier::Nym& nymID) const final;

#if OT_QT
    ui::AccountActivityQt* AccountActivityQt(
        const identifier::Nym& nymID,
        const Identifier& accountID) const final;
    ui::AccountListQt* AccountListQt(const identifier::Nym& nym) const final;
    ui::ActivitySummaryQt* ActivitySummaryQt(
        const identifier::Nym& nymID) const final;
    ui::ActivityThreadQt* ActivityThreadQt(
        const identifier::Nym& nymID,
        const Identifier& threadID) const final;
    QAbstractItemModel* BlankModel(const std::size_t columns) const final
    {
        return blank_.get(columns);
    }
    ui::ContactQt* ContactQt(const Identifier& contactID) const final;
    ui::ContactListQt* ContactListQt(const identifier::Nym& nymID) const final;
    ui::MessagableListQt* MessagableListQt(
        const identifier::Nym& nymID) const final;
    ui::PayableListQt* PayableListQt(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const final;
    ui::ProfileQt* ProfileQt(const identifier::Nym& nymID) const final;
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

#if OT_QT
    using AccountActivityValue = ui::AccountActivityQt;
    using AccountListValue = ui::AccountListQt;
    using AccountSummaryValue = std::unique_ptr<ui::AccountSummary>;
    using ActivitySummaryValue = ui::ActivitySummaryQt;
    using ActivityThreadValue = ui::ActivityThreadQt;
    using ContactValue = ui::ContactQt;
    using ContactListValue = ui::ContactListQt;
    using MessagableListValue = ui::MessagableListQt;
    using PayableListValue = ui::PayableListQt;
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
#endif  // OT_QT

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
    mutable AccountSummaryMap accounts_summaries_;
    mutable ActivitySummaryMap activity_summaries_;
    mutable ContactMap contacts_;
    mutable ContactListMap contact_lists_;
    mutable MessagableListMap messagable_lists_;
    mutable PayableListMap payable_lists_;
    mutable ActivityThreadMap activity_threads_;
    mutable ProfileMap profiles_;
#if OT_QT
    mutable Blank blank_;
#endif  // OT_QT
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
    MessagableListMap::mapped_type& messagable_list(
        const identifier::Nym& nymID) const;
    PayableListMap::mapped_type& payable_list(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const;
    ProfileMap::mapped_type& profile(const identifier::Nym& nymID) const;

    UI(const api::client::internal::Manager& api,
       const Flag& running
#if OT_QT
       ,
       const bool qt
#endif  // OT_QT
    );
    UI() = delete;
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};
}  // namespace opentxs::api::client::implementation
