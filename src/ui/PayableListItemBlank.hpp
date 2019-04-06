// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/PayableListItem.hpp"
#include "opentxs/ui/Widget.hpp"

#include "ContactListItemBlank.hpp"

namespace opentxs::ui::implementation
{
class PayableListItemBlank final : public PayableListRowInternal,
                                   public ContactListItemBlank
{
public:
    std::string PaymentCode() const override { return {}; }
    void SetCallback(ui::Widget::Callback) const override {}

    void reindex(const PayableListSortKey&, const implementation::CustomData&)
        override
    {
    }

    PayableListItemBlank() = default;
    ~PayableListItemBlank() = default;

private:
    PayableListItemBlank(const PayableListItemBlank&) = delete;
    PayableListItemBlank(PayableListItemBlank&&) = delete;
    PayableListItemBlank& operator=(const PayableListItemBlank&) = delete;
    PayableListItemBlank& operator=(PayableListItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
