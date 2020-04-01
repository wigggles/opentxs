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
    using TxidIndex = std::vector<Space>;
    using TransactionMap = std::map<ReadView, value_type>;

    auto at(const std::size_t index) const noexcept -> value_type final;
    auto at(const ReadView txid) const noexcept -> value_type final;
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
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
    auto size() const noexcept -> std::size_t final { return index_.size(); }

    Block(
        const api::internal::Core& api,
        const blockchain::Type chain,
        std::unique_ptr<const internal::Header> header,
        TxidIndex&& index,
        TransactionMap&& transactions) noexcept(false);

private:
    const std::unique_ptr<const internal::Header> header_p_;
    const internal::Header& header_;
    const TxidIndex index_;
    const TransactionMap transactions_;

    Block() = delete;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    Block& operator=(const Block&) = delete;
    Block& operator=(Block&&) = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
