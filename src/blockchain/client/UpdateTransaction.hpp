// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::client::implementation
{
class UpdateTransaction final : virtual public internal::UpdateTransaction
{
public:
    const BestHashes& BestChain() const { return reorg_; }
    const block::Position& Checkpoint() const { return checkpoint_; }
    const Segments& Connected() const { return connect_; }
    const Segments& Disconnected() const { return disconnected_; }
    bool HaveCheckpoint() const { return have_checkpoint_; }
    bool HaveReorg() const { return have_reorg_; }
    // throws std::out_of_range if header does not exist
    const block::Header& Header(const block::Hash& hash) const;
    bool IsInBestChain(const block::Position& position) const;
    bool IsInBestChain(const block::Height height, const block::Hash& hash)
        const;
    const block::Position& ReorgParent() const { return reorg_from_; }
    const Hashes& SiblingsToAdd() const { return add_sib_; }
    const Hashes& SiblingsToDelete() const { return delete_sib_; }
    const block::Position& Tip() const { return tip_; }
    bool TipIsBest() const { return is_best_; }

    void AddSibling(const block::Position& position);
    void AddToBestChain(const block::Position& position);
    void AddToBestChain(const block::Height height, const block::Hash& hash);
    void ClearCheckpoint();
    void ConnectBlock(ChainSegment&& segment);
    void DisconnectBlock(const block::Header& header);
    block::Header& ModifyExistingBlock(std::unique_ptr<block::Header> header);
    void RemoveSibling(const block::Hash& hash);
    void SetCheckpoint(block::Position&& checkpoint);
    void SetReorg(const bool value) { have_reorg_ = value; }
    void SetReorgParent(const block::Position& pos) { reorg_from_ = pos; }
    void SetTip(const block::Position& position) { tip_ = position; }
    void SetTipBest(const bool isBest) { is_best_ = isBest; }
    UpdatedHeader& UpdatedHeaders() { return headers_; }

private:
    friend opentxs::Factory;

    bool is_best_;
    bool have_reorg_;
    bool have_checkpoint_;
    block::Position tip_;
    block::Position reorg_from_;
    block::Position checkpoint_;
    UpdatedHeader headers_;
    BestHashes reorg_;
    Hashes add_sib_;
    Hashes delete_sib_;
    Segments connect_;
    Segments disconnected_;

    UpdateTransaction();
};
}  // namespace opentxs::blockchain::client::implementation
