// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "storage/tree/Node.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
class Driver;
}  // namespace storage
}  // namespace api

namespace storage
{
class Tree;

class BlockchainTransactions final : public Node
{
public:
    auto Load(
        const std::string& id,
        std::shared_ptr<proto::BlockchainTransaction>& output,
        const bool checking) const -> bool;
    auto LookupNyms(const std::string& txid) const -> std::set<OTNymID>;

    auto Delete(const std::string& id) -> bool;
    auto Store(
        const identifier::Nym& nym,
        const proto::BlockchainTransaction& data) -> bool;

    ~BlockchainTransactions() final = default;

private:
    friend Tree;

    using SerializedType = proto::StorageBlockchainTransactions;
    using NymIndexType = proto::StorageContactNymIndex;

    static const VersionNumber CurrentVersion{2};
    static const VersionNumber NymIndexVersion{1};

    std::map<std::string, std::set<OTNymID>> nym_index_;

    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> SerializedType;

    void init(const std::string& hash) final;

    BlockchainTransactions(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash);
    BlockchainTransactions() = delete;
    BlockchainTransactions(const BlockchainTransactions&) = delete;
    BlockchainTransactions(BlockchainTransactions&&) = delete;
    auto operator=(const BlockchainTransactions&)
        -> BlockchainTransactions = delete;
    auto operator=(BlockchainTransactions &&)
        -> BlockchainTransactions = delete;
};
}  // namespace storage
}  // namespace opentxs
