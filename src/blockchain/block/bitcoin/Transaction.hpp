// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Transaction final : public bitcoin::Transaction
{
public:
    static const VersionNumber default_version_;

    auto ExtractElements(const filter::Type style) const noexcept
        -> std::vector<Space> final;
    auto FindMatches(
        const FilterType type,
        const Patterns& txos,
        const Patterns& elements) const noexcept -> Matches final;
    auto ID() const noexcept -> const Txid& final { return txid_; }
    auto IDNormalized() const noexcept -> const Identifier&;
    auto Inputs() const noexcept -> const bitcoin::Inputs& final
    {
        return *inputs_;
    }
    auto Locktime() const noexcept -> std::uint32_t final { return lock_time_; }
    auto Outputs() const noexcept -> const bitcoin::Outputs& final
    {
        return *outputs_;
    }
    auto SegwitFlag() const noexcept -> std::byte final { return segwit_flag_; }
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize() const noexcept
        -> std::optional<proto::BlockchainTransaction> final;
    auto Version() const noexcept -> std::int32_t final { return version_; }
    auto WTXID() const noexcept -> const Txid& final { return wtxid_; }

    Transaction(
        const api::Core& api,
        const VersionNumber serializeVersion,
        const blockchain::Type chain,
        const std::int32_t version,
        const std::byte segwit,
        const std::uint32_t lockTime,
        const pTxid&& txid,
        const pTxid&& wtxid,
        std::unique_ptr<const bitcoin::Inputs> inputs,
        std::unique_ptr<const bitcoin::Outputs> outputs) noexcept(false);
    ~Transaction() final = default;

private:
    const api::Core& api_;
    const blockchain::Type chain_;
    const VersionNumber serialize_version_;
    const std::int32_t version_;
    const std::byte segwit_flag_;
    const std::uint32_t lock_time_;
    const pTxid txid_;
    const pTxid wtxid_;
    const std::unique_ptr<const bitcoin::Inputs> inputs_;
    const std::unique_ptr<const bitcoin::Outputs> outputs_;
    mutable std::optional<OTIdentifier> normalized_id_;
    mutable std::optional<std::size_t> size_;
    mutable std::optional<std::size_t> normalized_size_;

    auto calculate_size(const bool normalize) const noexcept -> std::size_t;
    auto serialize(const AllocateOutput destination, const bool normalize) const
        noexcept -> std::optional<std::size_t>;

    Transaction() = delete;
    Transaction(const Transaction&) = delete;
    Transaction(Transaction&&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    Transaction& operator=(Transaction&&) = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
