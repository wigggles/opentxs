// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_MESSAGABLELIST_HPP
#define OPENTXS_UI_MESSAGABLELIST_HPP

#ifndef Q_MOC_RUN
#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/ui/List.hpp"
#include "opentxs/SharedPimpl.hpp"

#ifdef SWIG
// clang-format off
%rename(UIMessagableList) opentxs::ui::MessagableList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class MessagableList;
}  // namespace implementation

class MessagableList : virtual public List
{
public:
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactListItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactListItem>
    Next() const noexcept = 0;

    OPENTXS_EXPORT ~MessagableList() override = default;

protected:
    MessagableList() noexcept = default;

private:
    MessagableList(const MessagableList&) = delete;
    MessagableList(MessagableList&&) = delete;
    MessagableList& operator=(const MessagableList&) = delete;
    MessagableList& operator=(MessagableList&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
class opentxs::ui::MessagableListQt final : public QIdentityProxyModel
{
    Q_OBJECT

signals:
    void updated() const;

public:
    // List layout
    enum Roles {
        ContactIDRole = Qt::UserRole + 0,
        SectionRole = Qt::UserRole + 1,
    };

    MessagableListQt(implementation::MessagableList& parent) noexcept;

    ~MessagableListQt() final = default;

private:
    friend opentxs::Factory;

    implementation::MessagableList& parent_;

    void notify() const noexcept;

    MessagableListQt() = delete;
    MessagableListQt(const MessagableListQt&) = delete;
    MessagableListQt(MessagableListQt&&) = delete;
    MessagableListQt& operator=(const MessagableListQt&) = delete;
    MessagableListQt& operator=(MessagableListQt&&) = delete;
};
#endif
#endif
