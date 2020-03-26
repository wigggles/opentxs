// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Outputs final : public bitcoin::Outputs
{
public:
    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return *outputs_.at(position);
    }
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto CalculateSize() const noexcept -> std::size_t final;
    auto cbegin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const noexcept -> const_iterator final
    {
        return const_iterator(this, outputs_.size());
    }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(proto::BlockchainTransaction& destination) const noexcept
        -> bool final;
    auto size() const noexcept -> std::size_t final { return outputs_.size(); }

    Outputs(
        std::vector<std::unique_ptr<bitcoin::Output>>&& outputs,
        std::optional<std::size_t> size = {}) noexcept(false);
    ~Outputs() final = default;

private:
    const std::vector<std::unique_ptr<bitcoin::Output>> outputs_;
    mutable std::optional<std::size_t> size_;

    Outputs() = delete;
    Outputs(const Outputs&) = delete;
    Outputs(Outputs&&) = delete;
    Outputs& operator=(const Outputs&) = delete;
    Outputs& operator=(Outputs&&) = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
