// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PAYABLELIST_HPP
#define OPENTXS_UI_PAYABLELIST_HPP

#ifndef Q_MOC_RUN
#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/ui/List.hpp"
#include "opentxs/SharedPimpl.hpp"

#ifdef SWIG
// clang-format off
%rename(UIPayableList) opentxs::ui::PayableList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class PayableList;
}  // namespace implementation

class PayableList;
class PayableListItem;

#if OT_QT
class PayableListQt;
#endif
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class PayableList : virtual public List
{
public:
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::PayableListItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::PayableListItem>
    Next() const noexcept = 0;

    OPENTXS_EXPORT ~PayableList() override = default;

protected:
    PayableList() noexcept = default;

private:
    PayableList(const PayableList&) = delete;
    PayableList(PayableList&&) = delete;
    PayableList& operator=(const PayableList&) = delete;
    PayableList& operator=(PayableList&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
class opentxs::ui::PayableListQt final : public QIdentityProxyModel
{
    Q_OBJECT

signals:
    void updated() const;

public:
    // Table layout: name, payment code
    enum Roles {
        ContactIDRole = Qt::UserRole + 0,
        SectionRole = Qt::UserRole + 1,
    };

    PayableListQt(implementation::PayableList& parent) noexcept;

    ~PayableListQt() final = default;

private:
    friend opentxs::Factory;

    implementation::PayableList& parent_;

    void notify() const noexcept;

    PayableListQt(const PayableListQt&) = delete;
    PayableListQt(PayableListQt&&) = delete;
    PayableListQt& operator=(const PayableListQt&) = delete;
    PayableListQt& operator=(PayableListQt&&) = delete;
};
#endif
#endif
