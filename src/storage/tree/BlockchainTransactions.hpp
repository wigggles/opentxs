// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/Editor.hpp"

#include "Node.hpp"

#include <cstdint>
#include <map>
#include <set>

namespace opentxs
{
namespace storage
{
class BlockchainTransactions final : public Node
{
public:
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::BlockchainTransaction>& output,
        const bool checking) const;
    std::set<OTNymID> LookupNyms(const std::string& txid) const;

    bool Delete(const std::string& id);
    bool Store(
        const identifier::Nym& nym,
        const proto::BlockchainTransaction& data);

    ~BlockchainTransactions() final = default;

private:
    friend Tree;

    using SerializedType = proto::StorageBlockchainTransactions;
    using NymIndexType = proto::StorageContactNymIndex;

    static const VersionNumber CurrentVersion{2};
    static const VersionNumber NymIndexVersion{1};

    std::map<std::string, std::set<OTNymID>> nym_index_;

    bool save(const std::unique_lock<std::mutex>& lock) const final;
    SerializedType serialize() const;

    void init(const std::string& hash) final;

    BlockchainTransactions(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash);
    BlockchainTransactions() = delete;
    BlockchainTransactions(const BlockchainTransactions&) = delete;
    BlockchainTransactions(BlockchainTransactions&&) = delete;
    BlockchainTransactions operator=(const BlockchainTransactions&) = delete;
    BlockchainTransactions operator=(BlockchainTransactions&&) = delete;
};
}  // namespace storage
}  // namespace opentxs
