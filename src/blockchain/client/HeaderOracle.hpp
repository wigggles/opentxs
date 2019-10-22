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
    bool AddHeaders(
        std::vector<std::unique_ptr<block::Header>>&) noexcept final;
    bool DeleteCheckpoint() noexcept final;

    ~HeaderOracle() final = default;

private:
    friend opentxs::Factory;

    struct Candidate {
        bool blacklisted_{false};
        std::vector<block::Position> chain_{};
    };

    using Candidates = std::vector<Candidate>;

    const api::internal::Core& api_;
    const internal::Network& network_;
    const internal::HeaderDatabase& database_;
    const blockchain::Type chain_;
    mutable std::mutex lock_;

    static bool evaluate_candidate(
        const block::Header& current,
        const block::Header& candidate) noexcept;

    block::Position best_chain(const Lock& lock) const noexcept;
    bool is_in_best_chain(const Lock& lock, const block::Hash& hash) const
        noexcept;

    bool add_header(
        const Lock& lock,
        UpdateTransaction& update,
        std::unique_ptr<block::Header> header) noexcept;
    bool apply_checkpoint(
        const Lock& lock,
        const block::Height height,
        UpdateTransaction& update) noexcept;
    std::pair<bool, bool> choose_candidate(
        const block::Header& current,
        const Candidates& candidates,
        UpdateTransaction& update) noexcept(false);
    void connect_children(
        const Lock& lock,
        block::Header& parentHeader,
        Candidates& candidates,
        Candidate& candidate,
        UpdateTransaction& update);
    // Returns true if the child is checkpoint blacklisted
    bool connect_to_parent(
        const Lock& lock,
        const UpdateTransaction& update,
        const block::Header& parent,
        block::Header& child) noexcept;
    Candidate& initialize_candidate(
        const Lock& lock,
        const block::Header& best,
        const block::Header& parent,
        UpdateTransaction& update,
        Candidates& candidates,
        block::Header& child,
        const block::Hash& stopHash = Data::Factory()) noexcept(false);
    const block::Header* is_disconnected(
        const block::Hash& parent,
        UpdateTransaction& update) noexcept;
    void stage_candidate(
        const Lock& lock,
        const block::Header& best,
        Candidates& candidates,
        UpdateTransaction& update,
        block::Header& child) noexcept(false);

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
