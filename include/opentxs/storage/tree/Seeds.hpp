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

#ifndef OPENTXS_STORAGE_TREE_SEEDS_HPP
#define OPENTXS_STORAGE_TREE_SEEDS_HPP

#include "opentxs/api/Editor.hpp"
#include "opentxs/storage/tree/Node.hpp"

#include <cstdint>

namespace opentxs
{
namespace storage
{

class Tree;

class Seeds : public Node
{
private:
    friend class Tree;

    std::string default_seed_;

    void init(const std::string& hash) override;
    bool save(const std::unique_lock<std::mutex>& lock) const override;
    void set_default(
        const std::unique_lock<std::mutex>& lock,
        const std::string& id);
    proto::StorageSeeds serialize() const;

    Seeds(const StorageDriver& storage, const std::string& hash);
    Seeds() = delete;
    Seeds(const Seeds&) = delete;
    Seeds(Seeds&&) = delete;
    Seeds operator=(const Seeds&) = delete;
    Seeds operator=(Seeds&&) = delete;

public:
    std::string Alias(const std::string& id) const;
    std::string Default() const;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Seed>& output,
        std::string& alias,
        const bool checking) const;

    bool Delete(const std::string& id);
    bool SetAlias(const std::string& id, const std::string& alias);
    bool SetDefault(const std::string& id);
    bool Store(const proto::Seed& data, const std::string& alias);

    ~Seeds() = default;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_SEEDS_HPP
