// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "ContactListItem.hpp"

namespace opentxs::ui::implementation
{
class PayableListItem final : public PayableListRowInternal,
                              public ContactListItem
{
public:
    std::string PaymentCode() const noexcept final;

    ~PayableListItem() = default;

private:
    friend opentxs::Factory;

    using ot_super = ContactListItem;

    std::string payment_code_{""};
    const proto::ContactItemType currency_;

    void reindex(
        const ContactListSortKey& key,
        const CustomData& custom) noexcept final;

    PayableListItem(
        const PayableInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const PayableListRowID& rowID,
        const PayableListSortKey& key,
        const std::string& paymentcode,
        const proto::ContactItemType& currency) noexcept;
    PayableListItem() = delete;
    PayableListItem(const PayableListItem&) = delete;
    PayableListItem(PayableListItem&&) = delete;
    PayableListItem& operator=(const PayableListItem&) = delete;
    PayableListItem& operator=(PayableListItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
