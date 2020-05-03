// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <optional>

#include "internal/blockchain/client/Client.hpp"
#include "opentxs/blockchain/Blockchain.hpp"

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
class Header;
}  // namespace block
}  // namespace blockchain

class Factory;
}  // namespace opentxs

namespace opentxs::blockchain::client
{
class UpdateTransaction
{
public:
    const BestHashes& BestChain() const { return best_; }
    block::Position Checkpoint() const;
    const Segments& Connected() const { return connect_; }
    const Segments& Disconnected() const { return disconnected_; }
    block::pHash EffectiveBestBlock(const block::Height height) const
        noexcept(false);
    bool EffectiveCheckpoint() const noexcept;
    const DisconnectedList& EffectiveDisconnectedHashes() const noexcept
    {
        return disconnected();
    }
    bool EffectiveHasDisconnectedChildren(const block::Hash& hash) const
        noexcept;
    bool EffectiveHeaderExists(const block::Hash& hash) const noexcept;
    bool EffectiveIsSibling(const block::Hash& hash) const noexcept
    {
        return 0 < siblings().count(hash);
    }
    const Hashes& EffectiveSiblingHashes() const noexcept { return siblings(); }
    bool HaveCheckpoint() const { return have_checkpoint_; }
    bool HaveReorg() const { return have_reorg_; }
    const block::Position& ReorgParent() const { return reorg_from_; }
    const Hashes& SiblingsToAdd() const { return add_sib_; }
    const Hashes& SiblingsToDelete() const { return delete_sib_; }
    const UpdatedHeader& UpdatedHeaders() const { return headers_; }

    void AddSibling(const block::Position& position);
    void AddToBestChain(const block::Position& position);
    void ClearCheckpoint();
    void ConnectBlock(ChainSegment&& segment);
    void DisconnectBlock(const block::Header& header);
    // throws std::out_of_range if header does not exist
    block::Header& Header(const block::Hash& hash) noexcept(false);
    void RemoveSibling(const block::Hash& hash);
    void SetCheckpoint(block::Position&& checkpoint);
    void SetReorgParent(const block::Position& pos) noexcept;
    // Stages best block for possible metadata update
    block::Header& Stage() noexcept;
    // Stages a brand new header
    block::Header& Stage(std::unique_ptr<block::Header> header) noexcept;
    // Stages an existing header for possible metadata update
    block::Header& Stage(const block::Hash& hash) noexcept(false);
    // Stages an existing header for possible metadata update
    block::Header& Stage(const block::Height& height) noexcept(false);

    UpdateTransaction(const api::Core& api, const internal::HeaderDatabase& db);

private:
    friend opentxs::Factory;

    const api::Core& api_;
    const internal::HeaderDatabase& db_;
    bool have_reorg_;
    bool have_checkpoint_;
    block::Position reorg_from_;
    block::Position checkpoint_;
    UpdatedHeader headers_;
    BestHashes best_;
    Hashes add_sib_;
    Hashes delete_sib_;
    Segments connect_;
    Segments disconnected_;
    mutable std::optional<DisconnectedList> cached_disconnected_;
    mutable std::optional<Hashes> cached_siblings_;

    DisconnectedList& disconnected() const noexcept;
    Hashes& siblings() const noexcept;
    block::Header& stage(
        const bool newHeader,
        std::unique_ptr<block::Header> header) noexcept;
};
}  // namespace opentxs::blockchain::client
