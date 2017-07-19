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

#ifndef OPENTXS_STORAGE_TREE_NYMS_HPP
#define OPENTXS_STORAGE_TREE_NYMS_HPP

#include "opentxs/api/Editor.hpp"
#include "opentxs/storage/tree/Node.hpp"
#include "opentxs/storage/Storage.hpp"

#include <map>
#include <mutex>
#include <string>
#include <tuple>

namespace opentxs
{
namespace storage
{

class Nym;
class Tree;

class Nyms : public Node
{
private:
    friend class Tree;

    mutable std::map<std::string, std::unique_ptr<class Nym>> nyms_;

    class Nym* nym(const std::string& id) const;
    void save(
        class Nym* nym,
        const std::unique_lock<std::mutex>& lock,
        const std::string& id);

    void init(const std::string& hash) override;
    bool save(const std::unique_lock<std::mutex>& lock) const override;
    proto::StorageNymList serialize() const;

    Nyms(const StorageDriver& storage, const std::string& hash);
    Nyms() = delete;
    Nyms(const Nyms&) = delete;
    Nyms(Nyms&&) = delete;
    Nyms operator=(const Nyms&) = delete;
    Nyms operator=(Nyms&&) = delete;

public:
    void Map(NymLambda lambda) const;
    const class Nym& Nym(const std::string& id) const;

    Editor<class Nym> mutable_Nym(const std::string& id);

    bool Migrate(const StorageDriver& to) const override;

    ~Nyms() = default;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_NYMS_HPP
