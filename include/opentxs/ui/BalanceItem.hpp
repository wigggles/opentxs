// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_BALANCEITEM_HPP
#define OPENTXS_UI_BALANCEITEM_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Types.hpp"

#include <cstdint>
#include <chrono>
#include <list>
#include <string>

#include "ListRow.hpp"

#ifdef SWIG
// clang-format off
%extend opentxs::ui::BalanceItem {
    int Timestamp() const
    {
        return std::chrono::system_clock::to_time_t($self->Timestamp());
    }
}
%ignore opentxs::ui::BalanceItem::Timestamp;
%ignore opentxs::ui::BalanceItem::Update;
%template(ListOfContactIDs) std::vector<std::string>;
%template(OTUIBalanceItem) opentxs::SharedPimpl<opentxs::ui::BalanceItem>;
%rename(UIBalanceItem) opentxs::ui::BalanceItem;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class BalanceItem : virtual public ListRow
{
public:
    EXPORT virtual opentxs::Amount Amount() const = 0;
    EXPORT virtual std::vector<std::string> Contacts() const = 0;
    EXPORT virtual std::string DisplayAmount() const = 0;
    EXPORT virtual std::string Memo() const = 0;
    EXPORT virtual std::string Workflow() const = 0;
    EXPORT virtual std::string Text() const = 0;
    EXPORT virtual std::chrono::system_clock::time_point Timestamp() const = 0;
    EXPORT virtual StorageBox Type() const = 0;

    EXPORT virtual ~BalanceItem() = default;

protected:
    BalanceItem() = default;

private:
    BalanceItem(const BalanceItem&) = delete;
    BalanceItem(BalanceItem&&) = delete;
    BalanceItem& operator=(const BalanceItem&) = delete;
    BalanceItem& operator=(BalanceItem&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif
