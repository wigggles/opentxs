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

#include "opentxs/Version.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/storage/tree/Node.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>

namespace opentxs
{
namespace api
{

class Storage;

}  // namespace api

namespace storage
{

class BlockchainTransactions;
class Contacts;
class Credentials;
class Nyms;
class Root;
class Seeds;
class Servers;
class Units;

class Tree : public Node
{
private:
    friend class api::Storage;
    friend class Root;

    std::string blockchain_root_{Node::BLANK_HASH};
    std::string contact_root_{Node::BLANK_HASH};
    std::string credential_root_{Node::BLANK_HASH};
    std::string nym_root_{Node::BLANK_HASH};
    std::string seed_root_{Node::BLANK_HASH};
    std::string server_root_{Node::BLANK_HASH};
    std::string unit_root_{Node::BLANK_HASH};

    mutable std::mutex blockchain_lock_;
    mutable std::unique_ptr<BlockchainTransactions> blockchain_;
    mutable std::mutex contact_lock_;
    mutable std::unique_ptr<Contacts> contacts_;
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

    BlockchainTransactions* blockchain() const;
    Contacts* contacts() const;
    Credentials* credentials() const;
    Nyms* nyms() const;
    Seeds* seeds() const;
    Servers* servers() const;
    Units* units() const;

    void save(BlockchainTransactions* blockchain, const Lock& lock);
    void save(Contacts* contacts, const Lock& lock);
    void save(Credentials* credentials, const Lock& lock);
    void save(Nyms* nyms, const Lock& lock);
    void save(Seeds* seeds, const Lock& lock);
    void save(Servers* servers, const Lock& lock);
    void save(Units* units, const Lock& lock);

    void init(const std::string& hash) override;
    bool save(const Lock& lock) const override;
    proto::StorageItems serialize() const;
    bool update_root(const std::string& hash);

    Tree(const opentxs::api::storage::Driver& storage, const std::string& key);
    Tree() = delete;
    Tree(const Tree&);
    Tree(Tree&&) = delete;
    Tree operator=(const Tree&) = delete;
    Tree operator=(Tree&&) = delete;

public:
    const BlockchainTransactions& BlockchainNode() const;
    const Contacts& ContactNode() const;
    const Credentials& CredentialNode() const;
    const Nyms& NymNode() const;
    const Seeds& SeedNode() const;
    const Servers& ServerNode() const;
    const Units& UnitNode() const;

    Editor<BlockchainTransactions> mutable_Blockchain();
    Editor<Contacts> mutable_Contacts();
    Editor<Credentials> mutable_Credentials();
    Editor<Nyms> mutable_Nyms();
    Editor<Seeds> mutable_Seeds();
    Editor<Servers> mutable_Servers();
    Editor<Units> mutable_Units();

    bool Migrate(const opentxs::api::storage::Driver& to) const override;

    ~Tree();
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_TREE_HPP
