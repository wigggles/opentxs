// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACTIVITYTHREAD_HPP
#define OPENTXS_UI_ACTIVITYTHREAD_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/ui/List.hpp"
#include "opentxs/Proto.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%extend opentxs::ui::ActivityThread {
    std::string PaymentCode(const int currency) const
    {
        return $self->PaymentCode(
            static_cast<opentxs::proto::ContactItemType>(currency));
    }
}
%ignore opentxs::ui::ActivityThread::PaymentCode;
%rename(UIActivityThread) opentxs::ui::ActivityThread;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class ActivityThread;
}  // namespace implementation

class ActivityThread : virtual public List
{
public:
    EXPORT virtual std::string DisplayName() const = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ActivityThreadItem> First()
        const = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ActivityThreadItem> Next()
        const = 0;
    EXPORT virtual std::string GetDraft() const = 0;
    EXPORT virtual std::string Participants() const = 0;
    EXPORT virtual bool Pay(
        const std::string& amount,
        const Identifier& sourceAccount,
        const std::string& memo = "",
        const PaymentType type = PaymentType::Cheque) const = 0;
    EXPORT virtual bool Pay(
        const Amount amount,
        const Identifier& sourceAccount,
        const std::string& memo = "",
        const PaymentType type = PaymentType::Cheque) const = 0;
    EXPORT virtual std::string PaymentCode(
        const proto::ContactItemType currency) const = 0;
    EXPORT virtual bool SendDraft() const = 0;
    EXPORT virtual bool SetDraft(const std::string& draft) const = 0;
    EXPORT virtual std::string ThreadID() const = 0;

    EXPORT virtual ~ActivityThread() = default;

protected:
    ActivityThread() = default;

private:
    ActivityThread(const ActivityThread&) = delete;
    ActivityThread(ActivityThread&&) = delete;
    ActivityThread& operator=(const ActivityThread&) = delete;
    ActivityThread& operator=(ActivityThread&&) = delete;
};

#if OT_QT
class ActivityThreadQt : public QAbstractItemModel
{
public:
    using ConstructorCallback = std::function<implementation::ActivityThread*(
        RowCallbacks insert,
        RowCallbacks remove)>;

    enum Roles {
        AmountPolarityRole = Qt::UserRole + 1,
        DisplayAmountRole = Qt::UserRole + 2,
        MemoRole = Qt::UserRole + 3,
        TextRole = Qt::UserRole + 4,
        TimestampRole = Qt::UserRole + 5,
        TypeRole = Qt::UserRole + 6,
        LoadingRole = Qt::UserRole + 7,
        PendingRole = Qt::UserRole + 8,
    };

    QString displayName() const;
    QString getDraft() const;
    QString participants() const;
    Q_INVOKABLE bool pay(
        const QString& amount,
        const QString& sourceAccount,
        const QString& memo = "") const;
    Q_INVOKABLE QString paymentCode(const int currency) const;
    Q_INVOKABLE bool sendDraft() const;
    Q_INVOKABLE bool setDraft(const QString& draft) const;
    QString threadID() const;

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

    const ActivityThread& operator*() const;

    ActivityThreadQt(ConstructorCallback cb);
    ~ActivityThreadQt() override;

signals:
    void updated() const;

private:
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName NOTIFY updated)
    Q_PROPERTY(QString draft READ getDraft NOTIFY updated)
    Q_PROPERTY(QString participants READ participants NOTIFY updated)
    Q_PROPERTY(QString threadID READ threadID NOTIFY updated)

    std::unique_ptr<implementation::ActivityThread> parent_;

    void notify() const;
    void finish_row_add();
    void finish_row_delete();
    void start_row_add(const QModelIndex& parent, int first, int last);
    void start_row_delete(const QModelIndex& parent, int first, int last);

    ActivityThreadQt() = delete;
    ActivityThreadQt(const ActivityThreadQt&) = delete;
    ActivityThreadQt(ActivityThreadQt&&) = delete;
    ActivityThreadQt& operator=(const ActivityThreadQt&) = delete;
    ActivityThreadQt& operator=(ActivityThreadQt&&) = delete;
};
#endif
}  // namespace ui
}  // namespace opentxs
#endif
