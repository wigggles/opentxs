// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountSummary.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/Contact.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "opentxs/ui/PayableList.hpp"
#include "opentxs/ui/Profile.hpp"
#include "opentxs/Types.hpp"

#include "internal/api/client/Client.hpp"
#include "ui/AccountActivity.hpp"
#include "ui/AccountList.hpp"
#include "ui/AccountSummary.hpp"
#include "ui/ActivitySummary.hpp"
#include "ui/ActivityThread.hpp"
#include "ui/Contact.hpp"
#include "ui/ContactList.hpp"
#include "ui/MessagableList.hpp"
#include "ui/PayableList.hpp"
#include "ui/Profile.hpp"

#include <map>
#include <memory>
#include <tuple>

#include "UI.hpp"

//#define OT_METHOD "opentxs::api::implementation::UI"

namespace opentxs
{
api::client::UI* Factory::UI(
    const api::client::internal::Manager& api,
    const Flag& running
#if OT_QT
    ,
    const bool qt
#endif
)
{
    return new api::client::implementation::UI(
        api,
        running
#if OT_QT
        ,
        qt
#endif
    );
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
UI::UI(
    const api::client::internal::Manager& api,
    const Flag& running
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    : api_(api)
    , running_(running)
#if OT_QT
    , enable_qt_(qt)
#endif
    , accounts_()
    , account_lists_()
    , account_summaries_()
    , activity_summaries_()
    , contacts_()
    , contact_lists_()
    , messagable_lists_()
    , payable_lists_()
    , activity_threads_()
    , profiles_()
#if OT_QT
    , blank_()
    , accounts_qt_()
    , account_lists_qt_()
    , account_summaries_qt_()
    , activity_summaries_qt_()
    , contacts_qt_()
    , contact_lists_qt_()
    , messagable_lists_qt_()
    , payable_lists_qt_()
    , activity_threads_qt_()
    , profiles_qt_()
#endif  // OT_QT
    , widget_update_publisher_(api_.ZeroMQ().PublishSocket())
{
    // WARNING: do not access api_.Wallet() during construction
    widget_update_publisher_->Start(api_.Endpoints().WidgetUpdate());
}

#if OT_QT
ui::BlankModel* UI::Blank::get(const std::size_t columns) noexcept
{
    Lock lock(lock_);

    {
        auto it = map_.find(columns);

        if (map_.end() != it) { return &(it->second); }
    }

    return &(map_.emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(columns),
                     std::forward_as_tuple(columns))
                 .first->second);
}
#endif  // OT_QT

auto UI::account_activity(
    const Lock& lock,
    const identifier::Nym& nymID,
    const Identifier& accountID) const noexcept
    -> AccountActivityMap::mapped_type&
{
    auto key = AccountActivityKey{nymID, accountID};
    auto it = accounts_.find(key);

    if (accounts_.end() == it) {
        it = accounts_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(
                         opentxs::Factory::AccountActivityModel(
                             api_,
                             widget_update_publisher_,
                             nymID,
                             accountID
#if OT_QT
                             ,
                             enable_qt_
#endif  // OT_QT
                             )))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

const ui::AccountActivity& UI::AccountActivity(
    const identifier::Nym& nymID,
    const Identifier& accountID) const noexcept
{
    Lock lock(lock_);

    return *account_activity(lock, nymID, accountID);
}

#if OT_QT
ui::AccountActivityQt* UI::AccountActivityQt(
    const identifier::Nym& nymID,
    const Identifier& accountID) const noexcept
{
    Lock lock(lock_);
    auto key = AccountActivityKey{nymID, accountID};
    auto it = accounts_qt_.find(key);

    if (accounts_qt_.end() == it) {
        it = accounts_qt_
                 .emplace(
                     std::move(key),
                     opentxs::Factory::AccountActivityQtModel(
                         *account_activity(lock, nymID, accountID)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}
#endif

auto UI::account_list(const Lock& lock, const identifier::Nym& nymID) const
    noexcept -> AccountListMap::mapped_type&
{
    auto key = AccountListKey{nymID};
    auto it = account_lists_.find(key);

    if (account_lists_.end() == it) {
        it = account_lists_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(opentxs::Factory::AccountListModel(
                         api_,
                         widget_update_publisher_,
                         nymID
#if OT_QT
                         ,
                         enable_qt_
#endif  // OT_QT
                         )))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

const ui::AccountList& UI::AccountList(const identifier::Nym& nymID) const
    noexcept
{
    Lock lock(lock_);

    return *account_list(lock, nymID);
}

#if OT_QT
ui::AccountListQt* UI::AccountListQt(const identifier::Nym& nymID) const
    noexcept
{
    Lock lock(lock_);
    auto key = AccountListKey{nymID};
    auto it = account_lists_qt_.find(key);

    if (account_lists_qt_.end() == it) {
        it = account_lists_qt_
                 .emplace(
                     std::move(key),
                     opentxs::Factory::AccountListQtModel(
                         *account_list(lock, nymID)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}
#endif

auto UI::account_summary(
    const Lock& lock,
    const identifier::Nym& nymID,
    const proto::ContactItemType currency) const noexcept
    -> AccountSummaryMap::mapped_type&
{
    auto key = AccountSummaryKey{nymID, currency};
    auto it = account_summaries_.find(key);

    if (account_summaries_.end() == it) {
        it =
            account_summaries_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(opentxs::Factory::AccountSummaryModel(
                        api_,
                        widget_update_publisher_,
                        nymID,
                        currency
#if OT_QT
                        ,
                        enable_qt_
#endif  // OT_QT
                        )))
                .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

const ui::AccountSummary& UI::AccountSummary(
    const identifier::Nym& nymID,
    const proto::ContactItemType currency) const noexcept
{
    Lock lock(lock_);

    return *account_summary(lock, nymID, currency);
}

#if OT_QT
ui::AccountSummaryQt* UI::AccountSummaryQt(
    const identifier::Nym& nymID,
    const proto::ContactItemType currency) const noexcept
{
    Lock lock(lock_);
    auto key = AccountSummaryKey{nymID, currency};
    auto it = account_summaries_qt_.find(key);

    if (account_summaries_qt_.end() == it) {
        it = account_summaries_qt_
                 .emplace(
                     std::move(key),
                     opentxs::Factory::AccountSummaryQtModel(
                         *account_summary(lock, nymID, currency)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}
#endif

auto UI::activity_summary(const Lock& lock, const identifier::Nym& nymID) const
    noexcept -> ActivitySummaryMap::mapped_type&
{
    auto key = ActivitySummaryKey{nymID};
    auto it = activity_summaries_.find(key);

    if (activity_summaries_.end() == it) {
        it = activity_summaries_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(
                         opentxs::Factory::ActivitySummaryModel(
                             api_,
                             widget_update_publisher_,
                             running_,
                             nymID
#if OT_QT
                             ,
                             enable_qt_
#endif  // OT_QT
                             )))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

const ui::ActivitySummary& UI::ActivitySummary(
    const identifier::Nym& nymID) const noexcept
{
    Lock lock(lock_);

    return *activity_summary(lock, nymID);
}

#if OT_QT
ui::ActivitySummaryQt* UI::ActivitySummaryQt(const identifier::Nym& nymID) const
    noexcept
{
    Lock lock(lock_);
    auto key = ActivitySummaryKey{nymID};
    auto it = activity_summaries_qt_.find(key);

    if (activity_summaries_qt_.end() == it) {
        it = activity_summaries_qt_
                 .emplace(
                     std::move(key),
                     opentxs::Factory::ActivitySummaryQtModel(
                         *activity_summary(lock, nymID)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}
#endif

auto UI::activity_thread(
    const Lock& lock,
    const identifier::Nym& nymID,
    const Identifier& threadID) const noexcept
    -> ActivityThreadMap::mapped_type&
{
    auto key = ActivityThreadKey{nymID, threadID};
    auto it = activity_threads_.find(key);

    if (activity_threads_.end() == it) {
        it =
            activity_threads_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(opentxs::Factory::ActivityThreadModel(
                        api_,
                        widget_update_publisher_,
                        nymID,
                        threadID
#if OT_QT
                        ,
                        enable_qt_
#endif  // OT_QT
                        )))
                .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

const ui::ActivityThread& UI::ActivityThread(
    const identifier::Nym& nymID,
    const Identifier& threadID) const noexcept
{
    Lock lock(lock_);

    return *activity_thread(lock, nymID, threadID);
}

#if OT_QT
ui::ActivityThreadQt* UI::ActivityThreadQt(
    const identifier::Nym& nymID,
    const Identifier& threadID) const noexcept
{
    Lock lock(lock_);
    auto key = ActivityThreadKey{nymID, threadID};
    auto it = activity_threads_qt_.find(key);

    if (activity_threads_qt_.end() == it) {
        it = activity_threads_qt_
                 .emplace(
                     std::move(key),
                     opentxs::Factory::ActivityThreadQtModel(
                         *activity_thread(lock, nymID, threadID)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}
#endif

auto UI::contact(const Lock& lock, const Identifier& contactID) const noexcept
    -> ContactMap::mapped_type&
{
    auto key = ContactKey{contactID};
    auto it = contacts_.find(key);

    if (contacts_.end() == it) {
        it = contacts_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(opentxs::Factory::ContactModel(
                         api_,
                         widget_update_publisher_,
                         contactID
#if OT_QT
                         ,
                         enable_qt_
#endif  // OT_QT
                         )))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

const ui::Contact& UI::Contact(const Identifier& contactID) const noexcept
{
    Lock lock(lock_);

    return *contact(lock, contactID);
}

#if OT_QT
ui::ContactQt* UI::ContactQt(const Identifier& contactID) const noexcept
{
    Lock lock(lock_);
    auto key = ContactKey{contactID};
    auto it = contacts_qt_.find(key);

    if (contacts_qt_.end() == it) {
        it =
            contacts_qt_
                .emplace(
                    std::move(key),
                    opentxs::Factory::ContactQtModel(*contact(lock, contactID)))
                .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}
#endif

auto UI::contact_list(const Lock& lock, const identifier::Nym& nymID) const
    noexcept -> ContactListMap::mapped_type&
{
    auto key = ContactListKey{nymID};
    auto it = contact_lists_.find(key);

    if (contact_lists_.end() == it) {
        it = contact_lists_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(opentxs::Factory::ContactListModel(
                         api_,
                         widget_update_publisher_,
                         nymID
#if OT_QT
                         ,
                         enable_qt_
#endif  // OT_QT
                         )))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

const ui::ContactList& UI::ContactList(const identifier::Nym& nymID) const
    noexcept
{
    Lock lock(lock_);

    return *contact_list(lock, nymID);
}

#if OT_QT
ui::ContactListQt* UI::ContactListQt(const identifier::Nym& nymID) const
    noexcept
{
    Lock lock(lock_);
    auto key = ContactListKey{nymID};
    auto it = contact_lists_qt_.find(key);

    if (contact_lists_qt_.end() == it) {
        it = contact_lists_qt_
                 .emplace(
                     std::move(key),
                     opentxs::Factory::ContactListQtModel(
                         *contact_list(lock, nymID)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}
#endif

auto UI::messagable_list(const Lock& lock, const identifier::Nym& nymID) const
    noexcept -> MessagableListMap::mapped_type&
{
    auto key = MessagableListKey{nymID};
    auto it = messagable_lists_.find(key);

    if (messagable_lists_.end() == it) {
        it =
            messagable_lists_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(opentxs::Factory::MessagableListModel(
                        api_,
                        widget_update_publisher_,
                        nymID
#if OT_QT
                        ,
                        enable_qt_
#endif  // OT_QT
                        )))
                .first;
    }

    return it->second;
}

const ui::MessagableList& UI::MessagableList(const identifier::Nym& nymID) const
    noexcept
{
    Lock lock(lock_);

    return *messagable_list(lock, nymID);
}

#if OT_QT
ui::MessagableListQt* UI::MessagableListQt(const identifier::Nym& nymID) const
    noexcept
{
    Lock lock(lock_);
    auto key = MessagableListKey{nymID};
    auto it = messagable_lists_qt_.find(key);

    if (messagable_lists_qt_.end() == it) {
        it = messagable_lists_qt_
                 .emplace(
                     std::move(key),
                     opentxs::Factory::MessagableListQtModel(
                         *messagable_list(lock, nymID)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}
#endif

auto UI::payable_list(
    const Lock& lock,
    const identifier::Nym& nymID,
    const proto::ContactItemType currency) const noexcept
    -> PayableListMap::mapped_type&
{
    auto key = PayableListKey{nymID, currency};
    auto it = payable_lists_.find(key);

    if (payable_lists_.end() == it) {
        it = payable_lists_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(opentxs::Factory::PayableListModel(
                         api_,
                         widget_update_publisher_,
                         nymID,
                         currency
#if OT_QT
                         ,
                         enable_qt_
#endif  // OT_QT
                         )))
                 .first;
    }

    return it->second;
}

const ui::PayableList& UI::PayableList(
    const identifier::Nym& nymID,
    proto::ContactItemType currency) const noexcept
{
    Lock lock(lock_);

    return *payable_list(lock, nymID, currency);
}

#if OT_QT
ui::PayableListQt* UI::PayableListQt(
    const identifier::Nym& nymID,
    proto::ContactItemType currency) const noexcept
{
    Lock lock(lock_);
    auto key = PayableListKey{nymID, currency};
    auto it = payable_lists_qt_.find(key);

    if (payable_lists_qt_.end() == it) {
        it = payable_lists_qt_
                 .emplace(
                     std::move(key),
                     opentxs::Factory::PayableListQtModel(
                         *payable_list(lock, nymID, currency)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}
#endif

auto UI::profile(const Lock& lock, const identifier::Nym& nymID) const noexcept
    -> ProfileMap::mapped_type&
{
    auto key = ProfileKey{nymID};
    auto it = profiles_.find(key);

    if (profiles_.end() == it) {
        it = profiles_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(opentxs::Factory::ProfileModel(
                         api_,
                         widget_update_publisher_,
                         nymID
#if OT_QT
                         ,
                         enable_qt_
#endif  // OT_QT
                         )))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

const ui::Profile& UI::Profile(const identifier::Nym& nymID) const noexcept
{
    Lock lock(lock_);

    return *profile(lock, nymID);
}

#if OT_QT
ui::ProfileQt* UI::ProfileQt(const identifier::Nym& nymID) const noexcept
{
    Lock lock(lock_);
    auto key = ProfileKey{nymID};
    auto it = profiles_qt_.find(key);

    if (profiles_qt_.end() == it) {
        it = profiles_qt_
                 .emplace(
                     std::move(key),
                     opentxs::Factory::ProfileQtModel(*profile(lock, nymID)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}
#endif
}  // namespace opentxs::api::client::implementation
