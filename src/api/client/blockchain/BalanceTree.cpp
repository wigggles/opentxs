// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Log.hpp"

#include "internal/api/client/blockchain/Blockchain.hpp"

#include "BalanceTree.tpp"

#define OT_METHOD                                                              \
    "opentxs::api::client::blockchain::implementation::BalanceTree::"

namespace opentxs
{
api::client::blockchain::internal::BalanceTree* Factory::BlockchainBalanceTree(
    const api::client::blockchain::internal::BalanceList& parent,
    const identifier::Nym& id,
    const std::set<OTIdentifier>& hdAccounts,
    [[maybe_unused]] const std::set<OTIdentifier>& importedAccounts,
    [[maybe_unused]] const std::set<OTIdentifier>& paymentCodeAccounts)
{
    using ReturnType = api::client::blockchain::implementation::BalanceTree;

    return new ReturnType(parent, id, hdAccounts);
}
}  // namespace opentxs

namespace opentxs::api::client::blockchain::implementation
{
BalanceTree::BalanceTree(
    const internal::BalanceList& parent,
    const identifier::Nym& nym,
    const std::set<OTIdentifier>& accounts) noexcept
    : api_(parent.API())
    , parent_(parent)
    , chain_(parent.Chain())
    , nym_id_(nym)
    , hd_(*this)
    , imported_(*this)
    , payment_code_(*this)
    , node_index_()
    , lock_()
    , unspent_()
    , spent_()
{
    init(accounts);
}

void BalanceTree::NodeIndex::Add(
    const std::string& id,
    internal::BalanceNode* node) noexcept
{
    OT_ASSERT(nullptr != node);

    Lock lock(lock_);
    index_[id] = node;
}

internal::BalanceNode* BalanceTree::NodeIndex::Find(const std::string& id) const
    noexcept
{
    Lock lock(lock_);

    try {

        return index_.at(id);
    } catch (...) {

        return nullptr;
    }
}

bool BalanceTree::AssociateTransaction(
    const std::vector<Activity>& unspent,
    const std::vector<Activity>& spent,
    std::set<OTIdentifier>& contacts,
    const PasswordPrompt& reason) const noexcept
{
    using ActivityVector = std::vector<Activity>;
    using ActivityPair = std::pair<ActivityVector, ActivityVector>;
    using ActivityMap = std::map<std::string, ActivityPair>;

    Lock lock(lock_);
    auto sorted = ActivityMap{};
    auto outputs = std::map<std::string, std::map<std::size_t, int>>{};

    for (const auto& [coin, key, amount] : unspent) {
        const auto& [transaction, output] = coin;
        const auto& [account, subchain, index] = key;

        if (1 < ++outputs[transaction][output]) { return false; }
        if (0 >= amount) { return false; }

        sorted[account].first.emplace_back(Activity{coin, key, amount});
    }

    for (const auto& [coin, key, amount] : spent) {
        const auto& [transaction, output] = coin;
        const auto& [account, subchain, index] = key;

        if (1 < ++outputs[transaction][output]) { return false; }
        if (0 >= amount) { return false; }

        sorted[account].second.emplace_back(Activity{coin, key, amount});
    }

    for (const auto& [accountID, value] : sorted) {
        auto* pNode = node_index_.Find(accountID);

        if (nullptr == pNode) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Account ")(accountID)(
                " not found")
                .Flush();

            continue;
        }

        const auto& node = *pNode;
        const auto accepted = node.AssociateTransaction(
            value.first, value.second, contacts, reason);

        if (accepted) {
            for (const auto& [coin, key, amount] : value.first) {
                unspent_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(coin),
                    std::forward_as_tuple(key, amount));
            }

            for (const auto& [coin, key, amount] : value.second) {
                spent_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(coin),
                    std::forward_as_tuple(key, amount));
            }
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed processing transaction")
                .Flush();

            return false;
        }
    }

    return true;
}

void BalanceTree::ClaimAccountID(
    const std::string& id,
    internal::BalanceNode* node) const noexcept
{
    node_index_.Add(id, node);
}

void BalanceTree::init(const std::set<OTIdentifier>& HDAccounts) noexcept
{
    for (const auto& accountID : HDAccounts) {
        std::shared_ptr<proto::HDAccount> account{};

        const auto loaded =
            api_.Storage().Load(nym_id_->str(), accountID->str(), account);

        if (false == loaded) { continue; }

        OT_ASSERT(account);

        auto notUsed = Identifier::Factory();
        hd_.Construct(*account, notUsed);
    }
}

std::optional<std::pair<Key, Amount>> BalanceTree::LookupUTXO(
    const Coin& coin) const noexcept
{
    Lock lock(lock_);

    try {

        return unspent_.at(coin);
    } catch (...) {

        return {};
    }
}

internal::BalanceNode& BalanceTree::Node(const Identifier& id) const
    noexcept(false)
{
    auto* output = node_index_.Find(id.str());

    if (nullptr == output) { throw std::out_of_range("Account not found"); }

    return *output;
}
}  // namespace opentxs::api::client::blockchain::implementation
