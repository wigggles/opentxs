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

#ifndef OPENTXS_STORAGE_TREE_TREE_HPP
#define OPENTXS_STORAGE_TREE_TREE_HPP

#include "opentxs/api/Editor.hpp"
#include "opentxs/storage/tree/Node.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>

namespace opentxs
{

class Storage;

namespace storage
{

class Credentials;
class Nyms;
class Root;
class Seeds;
class Servers;
class Units;

class Tree : public Node
{
private:
    friend class opentxs::Storage;
    friend class Root;

    std::string credential_root_;
    std::string nym_root_;
    std::string seed_root_;
    std::string server_root_;
    std::string unit_root_;

    mutable std::mutex credential_lock_;
    mutable std::unique_ptr<Credentials> credentials_;
    mutable std::mutex nym_lock_;
    mutable std::unique_ptr<Nyms> nyms_;
    mutable std::mutex seed_lock_;
    mutable std::unique_ptr<Seeds> seeds_;
    mutable std::mutex server_lock_;
    mutable std::unique_ptr<Servers> servers_;
    mutable std::mutex unit_lock_;
    mutable std::unique_ptr<Units> units_;

    Credentials* credentials() const;
    Nyms* nyms() const;
    Seeds* seeds() const;
    Servers* servers() const;
    Units* units() const;

    void save(
        Credentials* credentials,
        const std::unique_lock<std::mutex>& lock);
    void save(Nyms* nyms, const std::unique_lock<std::mutex>& lock);
    void save(Seeds* seeds, const std::unique_lock<std::mutex>& lock);
    void save(Servers* servers, const std::unique_lock<std::mutex>& lock);
    void save(Units* units, const std::unique_lock<std::mutex>& lock);

    void init(const std::string& hash) override;
    bool save(const std::unique_lock<std::mutex>& lock) const override;
    proto::StorageItems serialize() const;
    bool update_root(const std::string& hash);

    Tree(
        const Storage& storage,
        const keyFunction& migrate,
        const std::string& hash);
    Tree() = delete;
    Tree(const Tree&);
    Tree(Tree&&) = delete;
    Tree operator=(const Tree&) = delete;
    Tree operator=(Tree&&) = delete;

public:
    const Credentials& CredentialNode() const;
    const Nyms& NymNode() const;
    const Seeds& SeedNode() const;
    const Servers& ServerNode() const;
    const Units& UnitNode() const;

    Editor<Credentials> mutable_Credentials();
    Editor<Nyms> mutable_Nyms();
    Editor<Seeds> mutable_Seeds();
    Editor<Servers> mutable_Servers();
    Editor<Units> mutable_Units();

    bool Migrate() const override;

    ~Tree() = default;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_TREE_HPP
