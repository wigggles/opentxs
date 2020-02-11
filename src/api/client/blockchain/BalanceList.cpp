// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Log.hpp"

#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/api/client/Client.hpp"

#include "BalanceList.hpp"

namespace opentxs
{
api::client::blockchain::internal::BalanceList* Factory::BlockchainBalanceList(
    const api::client::internal::Blockchain& parent,
    const blockchain::Type chain)
{
    using ReturnType = api::client::blockchain::implementation::BalanceList;

    return new ReturnType(parent, chain);
}
}  // namespace opentxs

namespace opentxs::api::client::blockchain::implementation
{
BalanceList::BalanceList(
    const api::client::internal::Blockchain& parent,
    const opentxs::blockchain::Type chain) noexcept
    : parent_(parent)
    , api_(parent_.API())
    , chain_(chain)
    , lock_()
    , trees_()
    , index_()
{
    init();
}

bool BalanceList::add(
    const Lock& lock,
    const identifier::Nym& id,
    std::unique_ptr<internal::BalanceTree> tree) noexcept
{
    if (false == bool(tree)) { return false; }

    if (0 < index_.count(id)) { return false; }

    trees_.emplace_back(std::move(tree));
    const std::size_t position = trees_.size() - 1;
    index_.emplace(id, position);

    return true;
}

bool BalanceList::AddHDNode(
    const identifier::Nym& nym,
    const proto::HDPath& path,
    Identifier& id) noexcept
{
    Lock lock(lock_);

    return get_or_create(lock, nym).AddHDNode(path, id);
}

BalanceList::const_iterator::value_type& BalanceList::at(
    const std::size_t position) const noexcept(false)
{
    Lock lock(lock_);

    return at(lock, position);
}

const internal::BalanceTree& BalanceList::at(
    const Lock& lock,
    const std::size_t index) const noexcept(false)
{
    return *trees_.at(index);
}

internal::BalanceTree& BalanceList::at(
    const Lock& lock,
    const std::size_t index) noexcept(false)
{
    return *trees_.at(index);
}

std::unique_ptr<internal::BalanceTree> BalanceList::factory(
    const identifier::Nym& nym,
    const std::set<OTIdentifier>& accounts) const noexcept
{
    std::unique_ptr<internal::BalanceTree> output{
        opentxs::Factory::BlockchainBalanceTree(*this, nym, accounts, {}, {})};

    return output;
}

internal::BalanceTree& BalanceList::get_or_create(
    const Lock& lock,
    const identifier::Nym& id) noexcept
{
    if (0 == index_.count(id)) {
        auto pTree = factory(id, {});

        OT_ASSERT(pTree);

        const auto added = add(lock, id, std::move(pTree));

        OT_ASSERT(added);
    }

    return at(lock, index_.at(id));
}

void BalanceList::init() noexcept
{
    Lock lock(lock_);
    const auto nyms = api_.Storage().LocalNyms();

    for (const auto& id : nyms) {
        const auto accounts =
            api_.Storage().BlockchainAccountList(id, Translate(chain_));
        const auto nymID = identifier::Nym::Factory(id);
        std::set<OTIdentifier> accountIDs{};
        std::transform(
            accounts.begin(),
            accounts.end(),
            std::inserter(accountIDs, accountIDs.end()),
            [](const auto& in) -> OTIdentifier {
                return Identifier::Factory(in);
            });

        add(lock, nymID, factory(nymID, accountIDs));
    }
}

internal::BalanceTree& BalanceList::Nym(const identifier::Nym& id) noexcept
{
    Lock lock(lock_);

    return get_or_create(lock, id);
}
}  // namespace opentxs::api::client::blockchain::implementation
