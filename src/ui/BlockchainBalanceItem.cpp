// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "ui/BlockchainBalanceItem.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <string>

#include "internal/api/client/Client.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "ui/BalanceItem.hpp"
#include "ui/Widget.hpp"

// #define OT_METHOD "opentxs::ui::implementation::BlockchainBalanceItem::"

namespace opentxs::ui::implementation
{
BlockchainBalanceItem::BlockchainBalanceItem(
    const AccountActivityInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
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
    : BalanceItem(
          parent,
          api,
          publisher,
          rowID,
          sortKey,
          custom,
          nymID,
          accountID,
          text)
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
    const auto threads = api_.Storage().BlockchainThreadMap(nym_id_, txid_);
    auto output = std::vector<std::string>{};
    std::transform(
        std::begin(threads), std::end(threads), std::back_inserter(output), [
        ](const auto& id) -> auto { return id->str(); });

    return output;
}

auto BlockchainBalanceItem::DisplayAmount() const noexcept -> std::string
{
    return blockchain::internal::Format(chain_, amount_);
}
}  // namespace opentxs::ui::implementation
