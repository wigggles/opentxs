// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

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

#include "ui/AccountActivity.hpp"
#include "ui/AccountList.hpp"
#include "ui/ActivitySummary.hpp"
#include "ui/ActivityThread.hpp"
#include "ui/Contact.hpp"
#include "ui/ContactList.hpp"
#include "ui/Profile.hpp"

#include <map>
#include <memory>
#include <tuple>

#include "UI.hpp"

//#define OT_METHOD "opentxs::api::implementation::UI"

namespace opentxs
{
api::client::UI* Factory::UI(
    const api::client::Manager& api,
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
    const api::client::Manager& api,
    const Flag& running
#if OT_QT
    ,
    const bool qt
#endif
    )
    : api_(api)
    , running_(running)
#if OT_QT
    , enable_qt_(qt)
#endif
    , accounts_()
    , account_lists_()
    , accounts_summaries_()
    , activity_summaries_()
    , contacts_()
    , contact_lists_()
    , messagable_lists_()
    , widget_update_publisher_(api_.ZeroMQ().PublishSocket())
{
    // WARNING: do not access api_.Wallet() during construction
    widget_update_publisher_->Start(api_.Endpoints().WidgetUpdate());
}

UI::AccountActivityMap::mapped_type& UI::account_activity(
    const identifier::Nym& nymID,
    const Identifier& accountID) const
{
    Lock lock(lock_);
    AccountActivityKey key(nymID, accountID);
    auto it = accounts_.find(key);

    if (accounts_.end() == it) {
        it =
            accounts_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(
#if OT_QT
                        [&](RowCallbacks insert, RowCallbacks remove) -> auto* {
                            return new ui::implementation::AccountActivity(
                                api_,
                                widget_update_publisher_,
                                nymID,
                                accountID,
                                enable_qt_,
                                insert,
                                remove);
                        }
#else
                        new ui::implementation::AccountActivity(
                            api_, widget_update_publisher_, nymID, accountID)
#endif
                        ))
                .first;
    }

    return it->second;
}

const ui::AccountActivity& UI::AccountActivity(
    const identifier::Nym& nymID,
    const Identifier& accountID) const
{
    return *account_activity(nymID, accountID);
}

#if OT_QT
ui::AccountActivityQt* UI::AccountActivityQt(
    const identifier::Nym& nymID,
    const Identifier& accountID) const
{
    return &account_activity(nymID, accountID);
}
#endif

UI::AccountListMap::mapped_type& UI::account_list(
    const identifier::Nym& nymID) const
{
    Lock lock(lock_);
    AccountListKey key(nymID);
    auto it = account_lists_.find(key);

    if (account_lists_.end() == it) {
        it =
            account_lists_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(
#if OT_QT
                        [&](RowCallbacks insert, RowCallbacks remove) -> auto* {
                            return new ui::implementation::AccountList(
                                api_,
                                widget_update_publisher_,
                                nymID,
                                enable_qt_,
                                insert,
                                remove);
                        }
#else
                        new ui::implementation::AccountList(
                            api_, widget_update_publisher_, nymID)
#endif
                        ))
                .first;
    }

    return it->second;
}

const ui::AccountList& UI::AccountList(const identifier::Nym& nymID) const
{
    return *account_list(nymID);
}

#if OT_QT
ui::AccountListQt* UI::AccountListQt(const identifier::Nym& nymID) const
{
    return &account_list(nymID);
}
#endif

const ui::AccountSummary& UI::AccountSummary(
    const identifier::Nym& nymID,
    const proto::ContactItemType currency) const
{
    Lock lock(lock_);
    const AccountSummaryKey key{nymID, currency};
    auto& output = accounts_summaries_[key];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::AccountSummary(
            api_,
            widget_update_publisher_,
            nymID,
            currency
#if OT_QT
            ,
            enable_qt_
#endif
            ));
    }

    OT_ASSERT(output)

    return *output;
}

UI::ActivitySummaryMap::mapped_type& UI::activity_summary(
    const identifier::Nym& nymID) const
{
    Lock lock(lock_);
    ActivitySummaryKey key(nymID);
    auto it = activity_summaries_.find(key);

    if (activity_summaries_.end() == it) {
        it =
            activity_summaries_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(
#if OT_QT
                        [&](RowCallbacks insert, RowCallbacks remove) -> auto* {
                            return new ui::implementation::ActivitySummary(
                                api_,
                                widget_update_publisher_,
                                running_,
                                nymID,
                                enable_qt_,
                                insert,
                                remove);
                        }
#else
                        new ui::implementation::ActivitySummary(
                            api_, widget_update_publisher_, running_, nymID)
#endif
                        ))
                .first;
    }

    return it->second;
}

const ui::ActivitySummary& UI::ActivitySummary(
    const identifier::Nym& nymID) const
{
    return *activity_summary(nymID);
}

#if OT_QT
ui::ActivitySummaryQt* UI::ActivitySummaryQt(const identifier::Nym& nymID) const
{
    return &activity_summary(nymID);
}
#endif

UI::ActivityThreadMap::mapped_type& UI::activity_thread(
    const identifier::Nym& nymID,
    const Identifier& threadID) const
{
    Lock lock(lock_);
    ActivityThreadKey key(nymID, threadID);
    auto it = activity_threads_.find(key);

    if (activity_threads_.end() == it) {
        it =
            activity_threads_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(
#if OT_QT
                        [&](RowCallbacks insert, RowCallbacks remove) -> auto* {
                            return new ui::implementation::ActivityThread(
                                api_,
                                widget_update_publisher_,
                                nymID,
                                threadID,
                                enable_qt_,
                                insert,
                                remove);
                        }
#else
                        new ui::implementation::ActivityThread(
                            api_, widget_update_publisher_, nymID, threadID)
#endif
                        ))
                .first;
    }

    return it->second;
}

const ui::ActivityThread& UI::ActivityThread(
    const identifier::Nym& nymID,
    const Identifier& threadID) const
{
    return *activity_thread(nymID, threadID);
}

#if OT_QT
ui::ActivityThreadQt* UI::ActivityThreadQt(
    const identifier::Nym& nymID,
    const Identifier& threadID) const
{
    return &activity_thread(nymID, threadID);
}
#endif

UI::ContactMap::mapped_type& UI::contact(const Identifier& contactID) const
{
    Lock lock(lock_);
    ContactKey key(contactID);
    auto it = contacts_.find(key);

    if (contacts_.end() == it) {
        it =
            contacts_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(
#if OT_QT
                        [&](RowCallbacks insert, RowCallbacks remove) -> auto* {
                            return new ui::implementation::Contact(
                                api_,
                                widget_update_publisher_,
                                contactID,
                                enable_qt_,
                                insert,
                                remove);
                        }
#else
                        new ui::implementation::Contact(
                            api_, widget_update_publisher_, contactID)
#endif
                        ))
                .first;
    }

    return it->second;
}

const ui::Contact& UI::Contact(const Identifier& contactID) const
{
    return *contact(contactID);
}

#if OT_QT
ui::ContactQt* UI::ContactQt(const Identifier& contactID) const
{
    return &contact(contactID);
}
#endif

UI::ContactListMap::mapped_type& UI::contact_list(
    const identifier::Nym& nymID) const
{
    Lock lock(lock_);
    ContactListKey key(nymID);
    auto it = contact_lists_.find(key);

    if (contact_lists_.end() == it) {
        it =
            contact_lists_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(
#if OT_QT
                        [&](RowCallbacks insert, RowCallbacks remove) -> auto* {
                            return new ui::implementation::ContactList(
                                api_,
                                widget_update_publisher_,
                                nymID,
                                enable_qt_,
                                insert,
                                remove);
                        }
#else
                        new ui::implementation::ContactList(
                            api_, widget_update_publisher_, nymID)
#endif
                        ))
                .first;
    }

    return it->second;
}

const ui::ContactList& UI::ContactList(const identifier::Nym& nymID) const
{
    return *contact_list(nymID);
}

#if OT_QT
ui::ContactListQt* UI::ContactListQt(const identifier::Nym& nymID) const
{
    return &contact_list(nymID);
}
#endif

const ui::MessagableList& UI::MessagableList(const identifier::Nym& nymID) const
{
    Lock lock(lock_);
    auto& output = messagable_lists_[nymID];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::MessagableList(
            api_,
            widget_update_publisher_,
            nymID
#if OT_QT
            ,
            enable_qt_
#endif
            ));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::PayableList& UI::PayableList(
    const identifier::Nym& nymID,
    proto::ContactItemType currency) const
{
    Lock lock(lock_);
    auto& output = payable_lists_[std::pair<OTNymID, proto::ContactItemType>(
        nymID, currency)];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::PayableList(
            api_,
            widget_update_publisher_,
            nymID,
            currency
#if OT_QT
            ,
            enable_qt_
#endif
            ));
    }

    OT_ASSERT(output)

    return *output;
}

UI::ProfileMap::mapped_type& UI::profile(const identifier::Nym& nymID) const
{
    Lock lock(lock_);
    ProfileKey key(nymID);
    auto it = profiles_.find(key);

    if (profiles_.end() == it) {
        it =
            profiles_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(
#if OT_QT
                        [&](RowCallbacks insert, RowCallbacks remove) -> auto* {
                            return new ui::implementation::Profile(
                                api_,
                                widget_update_publisher_,
                                nymID,
                                enable_qt_,
                                insert,
                                remove);
                        }
#else
                        new ui::implementation::Profile(
                            api_, widget_update_publisher_, nymID)
#endif
                        ))
                .first;
    }

    return it->second;
}

const ui::Profile& UI::Profile(const identifier::Nym& nymID) const
{
    return *profile(nymID);
}

#if OT_QT
ui::ProfileQt* UI::ProfileQt(const identifier::Nym& nymID) const
{
    return &profile(nymID);
}
#endif
}  // namespace opentxs::api::client::implementation
