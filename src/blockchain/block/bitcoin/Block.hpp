// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace bb = opentxs::blockchain::bitcoin;

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Block final : public bitcoin::Block, public block::implementation::Block
{
public:
    using CalculatedSize =
        std::pair<std::size_t, blockchain::bitcoin::CompactSize>;
    using TxidIndex = std::vector<Space>;
    using TransactionMap = std::map<ReadView, value_type>;

    static const std::size_t header_bytes_;

    auto at(const std::size_t index) const noexcept -> const value_type& final;
    auto at(const ReadView txid) const noexcept -> const value_type& final;
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto CalculateSize() const noexcept -> std::size_t final
    {
        return calculate_size().first;
    }
    auto cbegin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const noexcept -> const_iterator final
    {
        return const_iterator(this, index_.size());
    }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto ExtractElements(const FilterType style) const noexcept
        -> std::vector<Space> final;
    auto FindMatches(
        const FilterType type,
        const Patterns& outpoints,
        const Patterns& scripts) const noexcept -> Matches final;
    auto Serialize(AllocateOutput bytes) const noexcept -> bool final;
    auto size() const noexcept -> std::size_t final { return index_.size(); }

    Block(
        const api::internal::Core& api,
        const blockchain::Type chain,
        std::unique_ptr<const internal::Header> header,
        TxidIndex&& index,
        TransactionMap&& transactions,
        std::optional<CalculatedSize>&& size = {}) noexcept(false);

private:
    static const value_type null_tx_;

    const std::unique_ptr<const internal::Header> header_p_;
    const internal::Header& header_;
    const TxidIndex index_;
    const TransactionMap transactions_;
    mutable std::optional<CalculatedSize> size_;

    auto calculate_size() const noexcept -> CalculatedSize;

    Block() = delete;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    Block& operator=(const Block&) = delete;
    Block& operator=(Block&&) = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
