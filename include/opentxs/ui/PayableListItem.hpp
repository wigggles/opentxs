// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PAYABLELISTITEM_HPP
#define OPENTXS_UI_PAYABLELISTITEM_HPP

#include "opentxs/Forward.hpp"

#include <string>

#include "ContactListItem.hpp"

#ifdef SWIG
// clang-format off
%template(OTUIPayableListItem) opentxs::SharedPimpl<opentxs::ui::PayableListItem>;
%rename(UIPayableListItem) opentxs::ui::PayableListItem;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class PayableListItem : virtual public ContactListItem
{
public:
    EXPORT virtual std::string PaymentCode() const noexcept = 0;

    EXPORT ~PayableListItem() override = default;

protected:
    PayableListItem() noexcept = default;

private:
    PayableListItem(const PayableListItem&) = delete;
    PayableListItem(PayableListItem&&) = delete;
    PayableListItem& operator=(const PayableListItem&) = delete;
    PayableListItem& operator=(PayableListItem&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif
