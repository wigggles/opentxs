// Copyright (c) 2018 The Open-Transactions developers
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

class Tree;

class BlockchainTransactions : public Node
{
public:
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::BlockchainTransaction>& output,
        const bool checking) const;

    bool Delete(const std::string& id);
    bool Store(const proto::BlockchainTransaction& data);

    ~BlockchainTransactions() = default;

private:
    friend class Tree;

    bool save(const std::unique_lock<std::mutex>& lock) const override;
    proto::StorageBlockchainTransactions serialize() const;

    void init(const std::string& hash) override;

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
