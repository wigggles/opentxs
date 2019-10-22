// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::client::implementation
{
class HeaderOracle final : virtual public internal::HeaderOracle
{
public:
    block::Position BestChain() const noexcept final;
    block::pHash BestHash(const block::Height height) const noexcept final;
    std::pair<block::Position, block::Position> CommonParent(
        const block::Position& position) const noexcept final;
    block::Position GetCheckpoint() const noexcept final;
    bool IsInBestChain(const block::Hash& hash) const noexcept final;
    std::unique_ptr<block::Header> LoadHeader(const block::Hash& hash) const
        noexcept final;
    std::vector<block::pHash> RecentHashes() const noexcept final
    {
        return database_.RecentHashes();
    }
    std::set<block::pHash> Siblings() const noexcept final;

    bool AddCheckpoint(
        const block::Height position,
        const block::Hash& requiredHash) noexcept final;
    bool AddHeader(std::unique_ptr<block::Header> header) noexcept final;
    bool DeleteCheckpoint() noexcept final;

    ~HeaderOracle() final = default;

private:
    friend opentxs::Factory;

    const api::internal::Core& api_;
    const internal::Network& network_;
    const internal::HeaderDatabase& database_;
    const blockchain::Type chain_;
    mutable std::mutex lock_;

    static bool evaluate_candidate(
        const block::Header& current,
        const block::Header& candidate) noexcept;

    block::Position best_chain(const Lock& lock) const noexcept;

    void blacklist_to_checkpoint(
        const Lock& lock,
        const block::Position& checkpoint,
        block::Header& header,
        internal::UpdateTransaction& update);
    bool calculate_reorg(
        const Lock& lock,
        const block::Header& header,
        internal::UpdateTransaction& update) noexcept;
    void connect_children(
        const Lock& lock,
        block::Header& parentHeader,
        std::vector<std::unique_ptr<block::Header>>& reconnectedTips,
        internal::UpdateTransaction& update);
    bool connect_to_parent(
        const Lock& lock,
        const block::Header& parent,
        block::Header& child) noexcept;
    bool insert_disconnected_block(
        const Lock& lock,
        std::unique_ptr<block::Header> header,
        std::unique_ptr<internal::UpdateTransaction> update) noexcept;
    bool is_in_best_chain(const Lock& lock, const block::Hash& hash) const
        noexcept;
    void reverse_blacklist(
        const Lock& lock,
        block::Header& header,
        internal::UpdateTransaction& update);
    bool reorg_to_checkpoint(
        const Lock& lock,
        const block::Position& checkpoint,
        internal::UpdateTransaction& update);
    block::Position scan_disconnected(
        const Lock& lock,
        const bool isCandidate,
        block::Header& parent,
        internal::UpdateTransaction& update,
        std::unique_ptr<const block::Header>& candidate);
    bool scan_for_checkpoint(
        const Lock& lock,
        const block::Position& checkpoint,
        const block::Header& header);

    HeaderOracle(
        const api::internal::Core& api,
        const internal::Network& network,
        const internal::HeaderDatabase& database,
        const blockchain::Type type) noexcept;
    HeaderOracle() = delete;
    HeaderOracle(const HeaderOracle&) = delete;
    HeaderOracle(HeaderOracle&&) = delete;
    HeaderOracle& operator=(const HeaderOracle&) = delete;
    HeaderOracle& operator=(HeaderOracle&&) = delete;
};
}  // namespace opentxs::blockchain::client::implementation
