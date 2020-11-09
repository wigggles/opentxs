// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/accountactivity/BlockchainBalanceItem.hpp"  // IWYU pragma: associated

#include <string>

#include "internal/api/client/Client.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/protobuf/PaymentEvent.pb.h"
#include "opentxs/protobuf/PaymentWorkflow.pb.h"
#include "ui/accountactivity/BalanceItem.hpp"
#include "ui/base/Widget.hpp"

// #define OT_METHOD "opentxs::ui::implementation::BlockchainBalanceItem::"

namespace opentxs::ui::implementation
{
BlockchainBalanceItem::BlockchainBalanceItem(
    const AccountActivityInternalInterface& parent,
    const api::client::internal::Manager& api,
    const AccountActivityRowID& rowID,
    const AccountActivitySortKey& sortKey,
    CustomData& custom,
    const identifier::Nym& nymID,
    const Identifier& accountID,
    [[maybe_unused]] const blockchain::Type chain,
    const OTData txid,
    const opentxs::Amount amount,
    const std::string memo,
    const std::string text) noexcept
    : BalanceItem(parent, api, rowID, sortKey, custom, nymID, accountID, text)
    , chain_(chain)
    , txid_(txid)
    , amount_(amount)
    , memo_(memo)
{
    extract_custom<proto::PaymentWorkflow>(custom, 0);
    extract_custom<proto::PaymentEvent>(custom, 1);
}

auto BlockchainBalanceItem::Contacts() const noexcept
    -> std::vector<std::string>
{
    auto output = std::vector<std::string>{};

    for (const auto& id : api_.Storage().BlockchainThreadMap(nym_id_, txid_)) {
        if (0 < id->size()) { output.emplace_back(id->str()); }
    }

    return output;
}

auto BlockchainBalanceItem::DisplayAmount() const noexcept -> std::string
{
    return blockchain::internal::Format(chain_, amount_);
}
}  // namespace opentxs::ui::implementation
