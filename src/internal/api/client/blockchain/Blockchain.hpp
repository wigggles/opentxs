// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>

#include "internal/blockchain/client/Client.hpp"
#include "opentxs/api/client/blockchain/BalanceList.hpp"
#include "opentxs/api/client/blockchain/BalanceNode.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/api/client/blockchain/Deterministic.hpp"
#include "opentxs/api/client/blockchain/HD.hpp"
#include "opentxs/api/client/blockchain/Imported.hpp"
#include "opentxs/api/client/blockchain/PaymentCode.hpp"

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

#if OT_BLOCKCHAIN
namespace opentxs::api::client::blockchain
{
using Address = opentxs::blockchain::p2p::internal::Address;
using Address_p = std::unique_ptr<Address>;
using Chain = opentxs::blockchain::Type;
using FilterData =
    opentxs::blockchain::client::internal::FilterDatabase::Filter;
using FilterHash = opentxs::blockchain::client::internal::FilterDatabase::Hash;
using FilterHeader =
    opentxs::blockchain::client::internal::FilterDatabase::Header;
using FilterType = opentxs::blockchain::filter::Type;
using Position = opentxs::blockchain::block::Position;
using Protocol = opentxs::blockchain::p2p::Protocol;
using Service = opentxs::blockchain::p2p::Service;
using Type = opentxs::blockchain::p2p::Network;
using UpdatedHeader = opentxs::blockchain::client::UpdatedHeader;

enum Table {
    BlockHeaders = 0,
    PeerDetails = 1,
    PeerChainIndex = 2,
    PeerProtocolIndex = 3,
    PeerServiceIndex = 4,
    PeerNetworkIndex = 5,
    PeerConnectedIndex = 6,
    FiltersBasic = 7,
    FiltersBCH = 8,
    FiltersOpentxs = 9,
    FilterHeadersBasic = 10,
    FilterHeadersBCH = 11,
    FilterHeadersOpentxs = 12,
    Config = 13,
    BlockIndex = 14,
};
}  // namespace opentxs::api::client::blockchain
#endif  // OT_BLOCKCHAIN

namespace opentxs::api::client::blockchain::internal
{
using ActivityMap = std::map<Coin, std::pair<blockchain::Key, Amount>>;

struct BalanceList : virtual public blockchain::BalanceList {
    virtual const api::internal::Core& API() const noexcept = 0;

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
    virtual const api::internal::Core& API() const noexcept = 0;
    virtual bool AssociateTransaction(
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
