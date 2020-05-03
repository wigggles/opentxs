// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/client/blockchain/BalanceList.cpp"

#pragma once

#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/blockchain/BalanceList.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace proto
{
class HDPath;
}  // namespace proto

class Factory;
}  // namespace opentxs

namespace opentxs::api::client::blockchain::implementation
{
class BalanceList final : virtual public internal::BalanceList
{
public:
    const api::internal::Core& API() const noexcept { return api_; }
    const_iterator::value_type& at(const std::size_t position) const
        noexcept(false) final;
    const_iterator begin() const noexcept final
    {
        return const_iterator(this, 0);
    }
    const_iterator cbegin() const noexcept final
    {
        return const_iterator(this, 0);
    }
    const_iterator cend() const noexcept final
    {
        return const_iterator(this, trees_.size());
    }
    opentxs::blockchain::Type Chain() const noexcept final { return chain_; }
    const_iterator end() const noexcept final
    {
        return const_iterator(this, trees_.size());
    }
    const client::internal::Blockchain& Parent() const noexcept final
    {
        return parent_;
    }
    std::size_t size() const noexcept final { return trees_.size(); }

    bool AddHDNode(
        const identifier::Nym& nym,
        const proto::HDPath& path,
        Identifier& id) noexcept final;
    internal::BalanceTree& Nym(const identifier::Nym& id) noexcept final;

    ~BalanceList() final = default;

private:
    friend opentxs::Factory;

    const client::internal::Blockchain& parent_;
    const api::internal::Core& api_;
    const opentxs::blockchain::Type chain_;
    mutable std::mutex lock_;
    std::vector<std::unique_ptr<internal::BalanceTree>> trees_;
    std::map<OTNymID, std::size_t> index_;

    using blockchain::BalanceList::at;
    const internal::BalanceTree& at(const Lock& lock, const std::size_t index)
        const noexcept(false);
    std::unique_ptr<internal::BalanceTree> factory(
        const identifier::Nym& nym,
        const std::set<OTIdentifier>& accounts) const noexcept;
    using blockchain::BalanceList::size;
    std::size_t size(const Lock& lock) const noexcept;

    bool add(
        const Lock& lock,
        const identifier::Nym& id,
        std::unique_ptr<internal::BalanceTree> tree) noexcept;
    internal::BalanceTree& at(
        const Lock& lock,
        const std::size_t index) noexcept(false);
    internal::BalanceTree& get_or_create(
        const Lock& lock,
        const identifier::Nym& id) noexcept;
    void init() noexcept;

    BalanceList(
        const api::client::internal::Blockchain& parent,
        const opentxs::blockchain::Type chain) noexcept;
    BalanceList(const BalanceList&) = delete;
    BalanceList(BalanceList&&) = delete;
    BalanceList& operator=(const BalanceList&) = delete;
    BalanceList& operator=(BalanceList&&) = delete;
};
}  // namespace opentxs::api::client::blockchain::implementation
