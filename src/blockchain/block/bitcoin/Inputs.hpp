// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <memory>
#include <optional>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"

namespace opentxs
{
namespace proto
{
class BlockchainTransaction;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Inputs final : public bitcoin::Inputs
{
public:
    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return *inputs_.at(position);
    }
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto CalculateSize(const bool normalized) const noexcept
        -> std::size_t final;
    auto cbegin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const noexcept -> const_iterator final
    {
        return const_iterator(this, inputs_.size());
    }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto ExtractElements(const filter::Type style) const noexcept
        -> std::vector<Space> final;
    auto FindMatches(
        const ReadView txid,
        const FilterType type,
        const Patterns& txos,
        const Patterns& elements) const noexcept -> Matches final;
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(proto::BlockchainTransaction& destination) const noexcept
        -> bool final;
    auto SerializeNormalized(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto size() const noexcept -> std::size_t final { return inputs_.size(); }

    Inputs(
        std::vector<std::unique_ptr<bitcoin::Input>>&& inputs,
        std::optional<std::size_t> size = {}) noexcept(false);
    ~Inputs() final = default;

private:
    const std::vector<std::unique_ptr<bitcoin::Input>> inputs_;
    mutable std::optional<std::size_t> size_;
    mutable std::optional<std::size_t> normalized_size_;

    auto serialize(const AllocateOutput destination, const bool normalize) const
        noexcept -> std::optional<std::size_t>;

    Inputs() = delete;
    Inputs(const Inputs&) = delete;
    Inputs(Inputs&&) = delete;
    Inputs& operator=(const Inputs&) = delete;
    Inputs& operator=(Inputs&&) = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
