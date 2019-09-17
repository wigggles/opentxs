// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "ActivityThreadItem.hpp"

namespace opentxs::ui::implementation
{
class PaymentItem final : public ActivityThreadItem
{
public:
    opentxs::Amount Amount() const noexcept final;
    bool Deposit() const noexcept final;
    std::string DisplayAmount() const noexcept final;
    std::string Memo() const noexcept final;

    ~PaymentItem();

private:
    friend opentxs::Factory;

    std::string display_amount_;
    std::string memo_;
    opentxs::Amount amount_;
    std::unique_ptr<std::thread> load_;
    std::shared_ptr<const OTPayment> payment_;

    void load() noexcept;

    PaymentItem(
        const ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        const CustomData& custom) noexcept;
    PaymentItem() = delete;
    PaymentItem(const PaymentItem&) = delete;
    PaymentItem(PaymentItem&&) = delete;
    PaymentItem& operator=(const PaymentItem&) = delete;
    PaymentItem& operator=(PaymentItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
