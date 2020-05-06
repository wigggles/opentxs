// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_UI_HPP
#define OPENTXS_API_CLIENT_UI_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Proto.hpp"

#ifdef SWIG
// clang-format off
%extend opentxs::api::client::UI {
    const ui::AccountSummary& AccountSummary(
        const identifier::Nym& nymID,
        const int currency) const noexcept
    {
        return $self->AccountSummary(
            nymID,
            static_cast<opentxs::proto::ContactItemType>(currency));
    }
    const opentxs::ui::PayableList& PayableList(
        const identifier::Nym& nymID,
        const int currency) const noexcept
    {
        return $self->PayableList(
            nymID,
            static_cast<opentxs::proto::ContactItemType>(currency));
    }
}
%ignore opentxs::api::client::UI::AccountSummary;
%ignore opentxs::api::client::UI::PayableList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace api
{
namespace client
{
class UI
{
public:
    OPENTXS_EXPORT virtual const ui::AccountActivity& AccountActivity(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept = 0;
    OPENTXS_EXPORT virtual const ui::AccountList& AccountList(
        const identifier::Nym& nym) const noexcept = 0;
    OPENTXS_EXPORT virtual const ui::AccountSummary& AccountSummary(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const noexcept = 0;
    OPENTXS_EXPORT virtual const ui::ActivitySummary& ActivitySummary(
        const identifier::Nym& nymID) const noexcept = 0;
    OPENTXS_EXPORT virtual const ui::ActivityThread& ActivityThread(
        const identifier::Nym& nymID,
        const Identifier& threadID) const noexcept = 0;
    OPENTXS_EXPORT virtual const ui::Contact& Contact(
        const Identifier& contactID) const noexcept = 0;
    OPENTXS_EXPORT virtual const ui::ContactList& ContactList(
        const identifier::Nym& nymID) const noexcept = 0;
    OPENTXS_EXPORT virtual const ui::MessagableList& MessagableList(
        const identifier::Nym& nymID) const noexcept = 0;
    OPENTXS_EXPORT virtual const ui::PayableList& PayableList(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const noexcept = 0;
    OPENTXS_EXPORT virtual const ui::Profile& Profile(
        const identifier::Nym& nymID) const noexcept = 0;

#if OT_QT
    /// Caller does not own this pointer
    OPENTXS_EXPORT virtual ui::AccountActivityQt* AccountActivityQt(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept = 0;
    /// Caller does not own this pointer
    OPENTXS_EXPORT virtual ui::AccountListQt* AccountListQt(
        const identifier::Nym& nym) const noexcept = 0;
    /// Caller does not own this pointer
    OPENTXS_EXPORT virtual ui::AccountSummaryQt* AccountSummaryQt(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const noexcept = 0;
    /// Caller does not own this pointer
    OPENTXS_EXPORT virtual ui::ActivitySummaryQt* ActivitySummaryQt(
        const identifier::Nym& nymID) const noexcept = 0;
    /// Caller does not own this pointer
    OPENTXS_EXPORT virtual ui::ActivityThreadQt* ActivityThreadQt(
        const identifier::Nym& nymID,
        const Identifier& threadID) const noexcept = 0;
    /// Caller does not own this pointer
    OPENTXS_EXPORT virtual QAbstractItemModel* BlankModel(
        const std::size_t columns) const noexcept = 0;
    /// Caller does not own this pointer
    OPENTXS_EXPORT virtual ui::ContactQt* ContactQt(
        const Identifier& contactID) const noexcept = 0;
    /// Caller does not own this pointer
    OPENTXS_EXPORT virtual ui::ContactListQt* ContactListQt(
        const identifier::Nym& nymID) const noexcept = 0;
    /// Caller does not own this pointer
    OPENTXS_EXPORT virtual ui::MessagableListQt* MessagableListQt(
        const identifier::Nym& nymID) const noexcept = 0;
    /// Caller does not own this pointer
    OPENTXS_EXPORT virtual ui::PayableListQt* PayableListQt(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const noexcept = 0;
    /// Caller does not own this pointer
    OPENTXS_EXPORT virtual ui::ProfileQt* ProfileQt(
        const identifier::Nym& nymID) const noexcept = 0;
#endif

    virtual ~UI() = default;

protected:
    UI() noexcept = default;

private:
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
