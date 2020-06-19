// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "ui/BlockchainActivityThreadItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/api/client/Client.hpp"
#include "ui/Widget.hpp"
#if OT_BLOCKCHAIN
#include "internal/blockchain/Blockchain.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "ui/ActivityThreadItem.hpp"

// #define OT_METHOD
// "opentxs::ui::implementation::BlockchainActivityThreadItem::"

namespace opentxs::factory
{
auto BlockchainActivityThreadItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>
{
    using ReturnType = ui::implementation::BlockchainActivityThreadItem;

    [[maybe_unused]] const auto chain =
        ui::implementation::extract_custom<blockchain::Type>(custom, 1);
    auto txid = ui::implementation::extract_custom<OTData>(custom, 2);
#if OT_BLOCKCHAIN
    const auto pTx = api.Blockchain().LoadTransactionBitcoin(txid->asHex());

    OT_ASSERT(pTx);

    const auto& tx = *pTx;
    const auto amount = tx.NetBalanceChange(nymID);
    const auto memo = tx.Memo();
    auto* text = static_cast<std::string*>(custom.at(0));

    OT_ASSERT(nullptr != text);

    *text = api.Blockchain().ActivityDescription(nymID, chain, tx);
#endif  // OT_BLOCKCHAIN

    return std::make_shared<ReturnType>(
        parent,
        api,
        publisher,
        nymID,
        rowID,
        sortKey,
        custom,
        std::move(txid),
#if OT_BLOCKCHAIN
        amount,
        blockchain::internal::Format(chain, amount),
        memo
#else
        Amount{},
        std::string{},
        std::string {}
#endif  // OT_BLOCKCHAIN
    );
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainActivityThreadItem::BlockchainActivityThreadItem(
    const ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    CustomData& custom,
    OTData&& txid,
    const opentxs::Amount amount,
    std::string&& displayAmount,
    const std::string& memo) noexcept
    : ActivityThreadItem(
          parent,
          api,
          publisher,
          nymID,
          rowID,
          sortKey,
          custom,
          false,
          false)
    , txid_(std::move(txid))
    , display_amount_(std::move(displayAmount))
    , memo_(memo)
    , amount_(amount)
{
    OT_ASSERT(false == nym_id_.empty())
    OT_ASSERT(false == item_id_.empty())
    OT_ASSERT(false == txid_->empty())
}
}  // namespace opentxs::ui::implementation
