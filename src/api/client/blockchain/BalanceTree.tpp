// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "BalanceTree.hpp"  // IWYU pragma: associated

#include "Factory.hpp"

namespace opentxs
{
namespace proto
{
class HDAccount;
class HDPath;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::api::client::blockchain::implementation
{
template <>
struct BalanceTree::Factory<internal::HD, proto::HDPath> {
    static internal::HD* get(
        const BalanceTree& parent,
        const proto::HDPath& data,
        Identifier& id) noexcept
    {
        return opentxs::Factory::BlockchainHDBalanceNode(parent, data, id);
    }
};
template <>
struct BalanceTree::Factory<internal::HD, proto::HDAccount> {
    static internal::HD* get(
        const BalanceTree& parent,
        const proto::HDAccount& data,
        Identifier& id) noexcept
    {
        return opentxs::Factory::BlockchainHDBalanceNode(parent, data, id);
    }
};

template <typename InterfaceType, typename PayloadType>
bool BalanceTree::NodeGroup<InterfaceType, PayloadType>::add(
    const Lock& lock,
    const Identifier& id,
    std::unique_ptr<PayloadType> node) noexcept
{
    if (false == bool(node)) {
        LogOutput("opentxs::api::client::blockchain::implementation::"
                  "BalanceTree::NodeGroup::")(__FUNCTION__)(": Invalid node")
            .Flush();

        return false;
    }

    if (0 < index_.count(id)) {
        LogOutput(
            "opentxs::api::client::blockchain::implementation::"
            "BalanceTree::NodeGroup::")(__FUNCTION__)(": Index already exists")
            .Flush();

        return false;
    }

    nodes_.emplace_back(std::move(node));
    const std::size_t position = nodes_.size() - 1;
    index_.emplace(id, position);

    return true;
}
}  // namespace opentxs::api::client::blockchain::implementation
