// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACT_HPP
#define OPENTXS_UI_CONTACT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/ui/List.hpp"
#include "opentxs/Proto.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%rename(UIContact) opentxs::ui::Contact;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class Contact;
}  // namespace implementation

class Contact : virtual public List
{
public:
    EXPORT virtual std::string ContactID() const = 0;
    EXPORT virtual std::string DisplayName() const = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactSection> First()
        const = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactSection> Next()
        const = 0;
    EXPORT virtual std::string PaymentCode() const = 0;

    EXPORT virtual ~Contact() = default;

protected:
    Contact() = default;

private:
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    Contact& operator=(const Contact&) = delete;
    Contact& operator=(Contact&&) = delete;
};

#if OT_QT
class ContactQt : public QAbstractItemModel
{
public:
    using ConstructorCallback = std::function<
        implementation::Contact*(RowCallbacks insert, RowCallbacks remove)>;

    enum Roles {};

    QString displayName() const;
    QString contactID() const;
    QString paymentCode() const;

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

    const Contact& operator*() const;

    ContactQt(ConstructorCallback cb);
    ~ContactQt() override;

signals:
    void updated() const;

private:
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName NOTIFY updated)
    Q_PROPERTY(QString contactID READ contactID NOTIFY updated)
    Q_PROPERTY(QString paymentCode READ paymentCode NOTIFY updated)

    std::unique_ptr<implementation::Contact> parent_;

    void notify() const;
    void finish_row_add();
    void finish_row_delete();
    void start_row_add(const QModelIndex& parent, int first, int last);
    void start_row_delete(const QModelIndex& parent, int first, int last);

    ContactQt() = delete;
    ContactQt(const ContactQt&) = delete;
    ContactQt(ContactQt&&) = delete;
    ContactQt& operator=(const ContactQt&) = delete;
    ContactQt& operator=(ContactQt&&) = delete;
};
#endif
}  // namespace ui
}  // namespace opentxs
#endif
