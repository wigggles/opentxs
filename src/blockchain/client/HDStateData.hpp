// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/client/blockchain/HD.hpp"

#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/Blockchain.hpp"

#include <atomic>
#include <map>
#include <optional>
#include <queue>
#include <vector>

namespace opentxs::blockchain::client::implementation
{
struct HDStateData {
    using OutstandingMap =
        std::map<block::pHash, BlockOracle::BitcoinBlockFuture>;
    using Subchain = internal::WalletDatabase::Subchain;
    using WalletDatabase = internal::WalletDatabase;

    const internal::Network& network_;
    const internal::WalletDatabase& db_;
    const api::client::blockchain::HD& node_;
    const filter::Type filter_type_;
    const Subchain subchain_;
    std::atomic<bool> running_;
    std::optional<Bip32Index> last_indexed_;
    std::optional<block::Position> last_scanned_;
    std::vector<block::pHash> blocks_to_request_;
    OutstandingMap outstanding_blocks_;
    std::queue<OutstandingMap::iterator> process_block_queue_;

    auto index() noexcept -> void;
    auto process() noexcept -> void;
    auto scan() noexcept -> void;

    HDStateData(
        const internal::Network& network,
        const WalletDatabase& db,
        const api::client::blockchain::HD& node,
        const filter::Type filter,
        const Subchain subchain) noexcept;
    HDStateData(HDStateData&&) noexcept;

private:
    auto get_targets(
        const internal::WalletDatabase::Patterns& keys,
        const std::vector<internal::WalletDatabase::UTXO>& unspent) const
        noexcept -> blockchain::internal::GCS::Targets;
    auto index_element(
        const filter::Type type,
        const api::client::blockchain::BalanceNode::Element& input,
        const Bip32Index index,
        WalletDatabase::ElementMap& output) noexcept -> void;
    auto update_utxos(
        const block::bitcoin::Block& block,
        const block::Position& position,
        const block::Block::Matches matches) noexcept -> void;

    HDStateData() = delete;
};
}  // namespace opentxs::blockchain::client::implementation
