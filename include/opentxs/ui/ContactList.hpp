// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTLIST_HPP
#define OPENTXS_UI_CONTACTLIST_HPP

#ifndef Q_MOC_RUN
#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIContactList) opentxs::ui::ContactList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class ContactList;
}  // namespace implementation

class ContactList;
class ContactListItem;

#if OT_QT
class ContactListQt;
#endif
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class ContactList : virtual public List
{
public:
    OPENTXS_EXPORT virtual std::string AddContact(
        const std::string& label,
        const std::string& paymentCode = "",
        const std::string& nymID = "") const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactListItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactListItem>
    Next() const noexcept = 0;

    OPENTXS_EXPORT ~ContactList() override = default;

protected:
    ContactList() noexcept = default;

private:
    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    ContactList& operator=(const ContactList&) = delete;
    ContactList& operator=(ContactList&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
class OPENTXS_EXPORT opentxs::ui::ContactListQt final
    : public QIdentityProxyModel
{
    Q_OBJECT

signals:
    OPENTXS_EXPORT void updated() const;

public:
    // List layout
    enum Roles {
        ContactIDRole = Qt::UserRole + 0,
        SectionRole = Qt::UserRole + 1,
    };

    OPENTXS_EXPORT Q_INVOKABLE QString addContact(
        const QString& label,
        const QString& paymentCode = "",
        const QString& nymID = "") const noexcept;

    ContactListQt(implementation::ContactList& parent) noexcept;

    ~ContactListQt() final = default;

private:
    friend opentxs::Factory;

    implementation::ContactList& parent_;

    void notify() const noexcept;

    ContactListQt() = delete;
    ContactListQt(const ContactListQt&) = delete;
    ContactListQt(ContactListQt&&) = delete;
    ContactListQt& operator=(const ContactListQt&) = delete;
    ContactListQt& operator=(ContactListQt&&) = delete;
};
#endif
#endif
