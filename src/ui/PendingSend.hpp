// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "ActivityThreadItem.hpp"

namespace opentxs::ui::implementation
{
class PendingSend final : public ActivityThreadItem
{
public:
    opentxs::Amount Amount() const noexcept final { return amount_; }
    bool Deposit() const noexcept final { return false; }
    std::string DisplayAmount() const noexcept final { return display_amount_; }
    std::string Memo() const noexcept final { return memo_; }

    ~PendingSend() = default;

private:
    friend opentxs::Factory;

    opentxs::Amount amount_;
    std::string display_amount_;
    std::string memo_;

    PendingSend(
        const ActivityThreadInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        const CustomData& custom) noexcept;
    PendingSend() = delete;
    PendingSend(const PendingSend&) = delete;
    PendingSend(PendingSend&&) = delete;
    PendingSend& operator=(const PendingSend&) = delete;
    PendingSend& operator=(PendingSend&&) = delete;
};
}  // namespace opentxs::ui::implementation
