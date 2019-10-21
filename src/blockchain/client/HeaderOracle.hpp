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
    bool AddCheckpoint(
        const block::Height position,
        const block::Hash& requiredHash) noexcept final;
    bool AddHeader(std::unique_ptr<block::Header> header) noexcept final;
    block::Position BestChain() noexcept final;
    block::pHash BestHash(const block::Height height) noexcept final;
    bool DeleteCheckpoint() noexcept final;
    block::Position GetCheckpoint() noexcept final;
    std::unique_ptr<block::Header> LoadHeader(
        const block::Hash& hash) noexcept final;
    std::set<block::pHash> Siblings() noexcept final;

    ~HeaderOracle() final = default;

private:
    friend opentxs::Factory;

    const api::internal::Core& api_;
    const internal::Network& network_;
    std::mutex lock_;

    static bool evaluate_candidate(
        const block::Header& current,
        const block::Header& candidate) noexcept;

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
        const blockchain::Type type) noexcept;
    HeaderOracle() = delete;
    HeaderOracle(const HeaderOracle&) = delete;
    HeaderOracle(HeaderOracle&&) = delete;
    HeaderOracle& operator=(const HeaderOracle&) = delete;
    HeaderOracle& operator=(HeaderOracle&&) = delete;
};
}  // namespace opentxs::blockchain::client::implementation
