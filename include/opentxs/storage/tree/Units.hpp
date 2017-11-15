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

#ifndef OPENTXS_STORAGE_TREE_UNITS_HPP
#define OPENTXS_STORAGE_TREE_UNITS_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/storage/tree/Node.hpp"

namespace opentxs
{
namespace storage
{

class Tree;

class Units : public Node
{
private:
    friend class Tree;

    void init(const std::string& hash) override;
    bool save(const std::unique_lock<std::mutex>& lock) const override;
    proto::StorageUnits serialize() const;

    Units(const opentxs::api::storage::Driver& storage, const std::string& key);
    Units() = delete;
    Units(const Units&) = delete;
    Units(Units&&) = delete;
    Units operator=(const Units&) = delete;
    Units operator=(Units&&) = delete;

public:
    std::string Alias(const std::string& id) const;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::UnitDefinition>& output,
        std::string& alias,
        const bool checking) const;
    void Map(UnitLambda lambda) const;

    bool Delete(const std::string& id);
    bool SetAlias(const std::string& id, const std::string& alias);
    bool Store(
        const proto::UnitDefinition& data,
        const std::string& alias,
        std::string& plaintext);

    ~Units() = default;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_UNITS_HPP
