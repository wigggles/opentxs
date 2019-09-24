// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/client/blockchain/BalanceList.hpp"
#include "opentxs/api/client/blockchain/BalanceNode.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/api/client/blockchain/Deterministic.hpp"
#include "opentxs/api/client/blockchain/HD.hpp"
#include "opentxs/api/client/blockchain/Imported.hpp"
#include "opentxs/api/client/blockchain/PaymentCode.hpp"

#include <optional>

namespace std
{
using COIN = std::tuple<std::string, std::size_t, std::int64_t>;

template <>
struct less<COIN> {
    bool operator()(const COIN& lhs, const COIN& rhs) const
    {
        /* TODO: these lines will cause a segfault in the clang ast parser.
                const auto & [ lID, lIndex, lAmount ] = lhs;
                const auto & [ rID, rIndex, rAmount ] = rhs;
        */

        const auto& lID = std::get<0>(lhs);
        const auto& lIndex = std::get<1>(lhs);
        const auto& lAmount = std::get<2>(lhs);
        const auto& rID = std::get<0>(rhs);
        const auto& rIndex = std::get<1>(rhs);
        const auto& rAmount = std::get<2>(rhs);

        if (lID < rID) { return true; }

        if (rID < lID) { return false; }

        if (lIndex < rIndex) { return true; }

        if (rIndex < lIndex) { return false; }

        return (lAmount < rAmount);
    }
};
}  // namespace std

namespace opentxs::api::client::blockchain::internal
{
using ActivityMap = std::map<Coin, std::pair<blockchain::Key, Amount>>;

struct BalanceList : virtual public blockchain::BalanceList {
    virtual const api::Core& API() const noexcept = 0;

    virtual bool AddHDNode(
        const identifier::Nym& nym,
        const proto::HDPath& path,
        Identifier& id) noexcept = 0;
    virtual BalanceTree& Nym(const identifier::Nym& id) noexcept = 0;
    virtual const client::internal::Blockchain& Parent() const noexcept = 0;
};

struct BalanceElement : virtual public blockchain::BalanceNode::Element {
    using SerializedType = proto::BlockchainAddress;

    virtual std::set<OTData> Elements() const noexcept = 0;
    virtual const Identifier& ID() const noexcept = 0;
    virtual std::set<std::string> IncomingTransactions() const noexcept = 0;
    virtual const identifier::Nym& NymID() const noexcept = 0;
    virtual SerializedType Serialize() const noexcept = 0;

    virtual void SetContact(const Identifier& id) noexcept = 0;
    virtual void SetLabel(const std::string& label) noexcept = 0;
    virtual void SetMetadata(
        const Identifier& contact,
        const std::string& label) noexcept = 0;
};

struct BalanceNode : virtual public blockchain::BalanceNode {
    virtual bool AssociateTransaction(
        const std::vector<Activity>& unspent,
        const std::vector<Activity>& spent,
        std::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept = 0;
    virtual const internal::BalanceElement& BalanceElement(
        const Subchain type,
        const Bip32Index index) const noexcept(false) = 0;
    virtual std::set<std::string> IncomingTransactions(const Key& key) const
        noexcept = 0;

    virtual bool SetContact(
        const Subchain type,
        const Bip32Index index,
        const Identifier& id) noexcept(false) = 0;
    virtual bool SetLabel(
        const Subchain type,
        const Bip32Index index,
        const std::string& label) noexcept(false) = 0;
};

struct BalanceTree : virtual public blockchain::BalanceTree {
    virtual const api::Core& API() const noexcept = 0;
    EXPORT virtual bool AssociateTransaction(
        const std::vector<Activity>& unspent,
        const std::vector<Activity>& spent,
        std::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept = 0;
    virtual opentxs::blockchain::Type Chain() const noexcept = 0;
    virtual void ClaimAccountID(
        const std::string& id,
        internal::BalanceNode* node) const noexcept = 0;
    virtual std::optional<std::pair<Key, Amount>> LookupUTXO(
        const Coin& coin) const noexcept = 0;
    virtual const blockchain::internal::HD& HDChain(
        const Identifier& account) const noexcept(false) = 0;
    virtual const internal::BalanceList& Parent() const noexcept = 0;

    virtual bool AddHDNode(
        const proto::HDPath& path,
        Identifier& id) noexcept = 0;
    virtual internal::BalanceNode& Node(const Identifier& id) const
        noexcept(false) = 0;
};

struct Deterministic : virtual public blockchain::Deterministic,
                       virtual public BalanceNode {
};

struct HD : virtual public blockchain::HD, virtual public Deterministic {
};

struct Imported : virtual public blockchain::Imported,
                  virtual public BalanceNode {
};

struct PaymentCode : virtual public blockchain::PaymentCode,
                     virtual public Deterministic {
};
}  // namespace opentxs::api::client::blockchain::internal
