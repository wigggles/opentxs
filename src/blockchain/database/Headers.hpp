// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <algorithm>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "api/client/blockchain/database/Database.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin

class Header;
}  // namespace block

namespace client
{
class UpdateTransaction;
}  // namespace client
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::database
{
struct Headers {
public:
    using Common = api::client::blockchain::database::implementation::Database;

    auto BestBlock(const block::Height position) const noexcept(false)
        -> block::pHash;
    auto CurrentBest() const noexcept -> std::unique_ptr<block::Header>
    {
        return load_header(best().second);
    }
    auto CurrentCheckpoint() const noexcept -> block::Position;
    auto DisconnectedHashes() const noexcept -> client::DisconnectedList;
    auto HasDisconnectedChildren(const block::Hash& hash) const noexcept
        -> bool;
    auto HaveCheckpoint() const noexcept -> bool;
    auto HeaderExists(const block::Hash& hash) const noexcept -> bool;
    void import_genesis(const blockchain::Type type) const noexcept;
    auto IsSibling(const block::Hash& hash) const noexcept -> bool;
    // Throws std::out_of_range if the header does not exist
    auto LoadHeader(const block::Hash& hash) const
        -> std::unique_ptr<block::Header>
    {
        return load_header(hash);
    }
    auto RecentHashes() const noexcept -> std::vector<block::pHash>;
    auto SiblingHashes() const noexcept -> client::Hashes;
    // Returns null pointer if the header does not exist
    auto TryLoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::bitcoin::Header>;
    // Returns null pointer if the header does not exist
    auto TryLoadHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::Header>;

    auto ApplyUpdate(const client::UpdateTransaction& update) noexcept -> bool;

    Headers(
        const api::Core& api,
        const client::internal::Network& network,
        const Common& common,
        const opentxs::storage::lmdb::LMDB& lmdb,
        const blockchain::Type type) noexcept;

private:
    const api::Core& api_;
    const client::internal::Network& network_;
    const Common& common_;
    const opentxs::storage::lmdb::LMDB& lmdb_;
    mutable std::mutex lock_;

    auto best() const noexcept -> block::Position;
    auto best(const Lock& lock) const noexcept -> block::Position;
    auto checkpoint(const Lock& lock) const noexcept -> block::Position;
    auto header_exists(const Lock& lock, const block::Hash& hash) const noexcept
        -> bool;
    // Throws std::out_of_range if the header does not exist
    auto load_bitcoin_header(const block::Hash& hash) const noexcept(false)
        -> std::unique_ptr<block::bitcoin::Header>;
    // Throws std::out_of_range if the header does not exist
    auto load_header(const block::Hash& hash) const noexcept(false)
        -> std::unique_ptr<block::Header>;
    auto pop_best(const std::size_t i, MDB_txn* parent) const noexcept -> bool;
    auto push_best(
        const block::Position next,
        const bool setTip,
        MDB_txn* parent) const noexcept -> bool;
    auto recent_hashes(const Lock& lock) const noexcept
        -> std::vector<block::pHash>;
};
}  // namespace opentxs::blockchain::database
