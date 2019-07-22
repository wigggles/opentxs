// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTLIST_HPP
#define OPENTXS_UI_CONTACTLIST_HPP

#ifndef Q_MOC_RUN

#include "opentxs/Forward.hpp"

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

class ContactList : virtual public List
{
public:
    EXPORT virtual std::string AddContact(
        const std::string& label,
        const std::string& paymentCode = "",
        const std::string& nymID = "") const = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactListItem> First()
        const = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactListItem> Next()
        const = 0;

    EXPORT virtual ~ContactList() = default;

protected:
    ContactList() = default;

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
class opentxs::ui::ContactListQt : public QAbstractItemModel
{
    Q_OBJECT

public:
    using ConstructorCallback = std::function<
        implementation::ContactList*(RowCallbacks insert, RowCallbacks remove)>;

    enum Roles {
        IDRole = Qt::UserRole + 1,
        NameRole = Qt::UserRole + 2,
        ImageRole = Qt::UserRole + 3,
        SectionRole = Qt::UserRole + 4,
    };

    Q_INVOKABLE QString addContact(
        const QString& label,
        const QString& paymentCode = "",
        const QString& nymID = "") const;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)
        const override;
    QModelIndex index(
        int row,
        int column,
        const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    const ContactList& operator*() const;

    ContactListQt(ConstructorCallback cb);
    ~ContactListQt() override;

signals:
    void updated() const;

private:
    std::unique_ptr<implementation::ContactList> parent_;

    void notify() const;
    void finish_row_add();
    void finish_row_delete();
    void start_row_add(const QModelIndex& parent, int first, int last);
    void start_row_delete(const QModelIndex& parent, int first, int last);

    ContactListQt() = delete;
    ContactListQt(const ContactListQt&) = delete;
    ContactListQt(ContactListQt&&) = delete;
    ContactListQt& operator=(const ContactListQt&) = delete;
    ContactListQt& operator=(ContactListQt&&) = delete;
};
#endif
#endif
