/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_STORAGE_TREE_BLOCKCHAIN_TRANSACTIONS_HPP
#define OPENTXS_STORAGE_TREE_BLOCKCHAIN_TRANSACTIONS_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/storage/tree/Node.hpp"

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
        const StorageDriver& storage,
        const std::string& hash);
    BlockchainTransactions() = delete;
    BlockchainTransactions(const BlockchainTransactions&) = delete;
    BlockchainTransactions(BlockchainTransactions&&) = delete;
    BlockchainTransactions operator=(const BlockchainTransactions&) = delete;
    BlockchainTransactions operator=(BlockchainTransactions&&) = delete;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_BLOCKCHAIN_TRANSACTIONS_HPP
