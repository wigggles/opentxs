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

#include "opentxs/stdafx.hpp"

#include "opentxs/storage/tree/Tree.hpp"

#include "opentxs/storage/tree/BlockchainTransactions.hpp"
#include "opentxs/storage/tree/Contacts.hpp"
#include "opentxs/storage/tree/Credentials.hpp"
#include "opentxs/storage/tree/Nym.hpp"
#include "opentxs/storage/tree/Nyms.hpp"
#include "opentxs/storage/tree/Seeds.hpp"
#include "opentxs/storage/tree/Servers.hpp"
#include "opentxs/storage/tree/Units.hpp"
#include "opentxs/storage/StoragePlugin.hpp"

namespace opentxs
{
namespace storage
{

#define CURRENT_VERSION 3

#define OT_METHOD "opentxs::storage::Tree::"

Tree::Tree(const StorageDriver& storage, const std::string& hash)
    : Node(storage, hash)
    , blockchain_root_(Node::BLANK_HASH)
    , contact_root_(Node::BLANK_HASH)
    , credential_root_(Node::BLANK_HASH)
    , nym_root_(Node::BLANK_HASH)
    , seed_root_(Node::BLANK_HASH)
    , server_root_(Node::BLANK_HASH)
    , unit_root_(Node::BLANK_HASH)
    , blockchain_lock_()
    , blockchain_(nullptr)
    , contact_lock_()
    , contacts_(nullptr)
    , credential_lock_()
    , credentials_(nullptr)
    , nym_lock_()
    , nyms_(nullptr)
    , seed_lock_()
    , seeds_(nullptr)
    , server_lock_()
    , servers_(nullptr)
    , unit_lock_()
    , units_(nullptr)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = CURRENT_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

Tree::Tree(const Tree& rhs)
    : Node(rhs.driver_, "")
    , blockchain_root_(Node::BLANK_HASH)
    , contact_root_(Node::BLANK_HASH)
    , credential_root_(Node::BLANK_HASH)
    , nym_root_(Node::BLANK_HASH)
    , seed_root_(Node::BLANK_HASH)
    , server_root_(Node::BLANK_HASH)
    , unit_root_(Node::BLANK_HASH)
    , blockchain_lock_()
    , blockchain_(nullptr)
    , contact_lock_()
    , contacts_(nullptr)
    , credential_lock_()
    , credentials_(nullptr)
    , nym_lock_()
    , nyms_(nullptr)
    , seed_lock_()
    , seeds_(nullptr)
    , server_lock_()
    , servers_(nullptr)
    , unit_lock_()
    , units_(nullptr)
{
    Lock lock(rhs.write_lock_);

    version_ = rhs.version_;
    root_ = rhs.root_;
    contact_root_ = rhs.contact_root_;
    credential_root_ = rhs.credential_root_;
    nym_root_ = rhs.nym_root_;
    seed_root_ = rhs.seed_root_;
    server_root_ = rhs.server_root_;
    unit_root_ = rhs.unit_root_;
}

BlockchainTransactions* Tree::blockchain() const
{
    Lock lock(blockchain_lock_);

    if (false == bool(blockchain_)) {
        blockchain_.reset(
            new BlockchainTransactions(driver_, blockchain_root_));

        if (false == bool(blockchain_)) {
            otErr << OT_METHOD << __FUNCTION__ << ": Unable to instantiate."
                  << std::endl;

            abort();
        }
    }

    lock.unlock();

    return blockchain_.get();
}

const BlockchainTransactions& Tree::BlockchainNode() const
{
    return *blockchain();
}

const Contacts& Tree::ContactNode() const { return *contacts(); }

Contacts* Tree::contacts() const
{
    Lock lock(contact_lock_);

    if (!contacts_) {
        contacts_.reset(new Contacts(driver_, contact_root_));

        if (!contacts_) {
            otErr << OT_METHOD << __FUNCTION__ << ": Unable to instantiate."
                  << std::endl;

            abort();
        }
    }

    lock.unlock();

    return contacts_.get();
}

const Credentials& Tree::CredentialNode() const { return *credentials(); }

Credentials* Tree::credentials() const
{
    Lock lock(credential_lock_);

    if (!credentials_) {
        credentials_.reset(new Credentials(driver_, credential_root_));

        if (!credentials_) {
            otErr << OT_METHOD << __FUNCTION__ << ": Unable to instantiate."
                  << std::endl;

            abort();
        }
    }

    lock.unlock();

    return credentials_.get();
}

void Tree::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageItems> serialized{nullptr};
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to load root index file." << std::endl;

        abort();
    }

    version_ = serialized->version();

    // Upgrade version
    if (CURRENT_VERSION > version_) {
        version_ = CURRENT_VERSION;
    }

    blockchain_root_ = normalize_hash(serialized->blockchaintransactions());
    contact_root_ = normalize_hash(serialized->contacts());
    credential_root_ = normalize_hash(serialized->creds());
    nym_root_ = normalize_hash(serialized->nyms());
    seed_root_ = normalize_hash(serialized->seeds());
    server_root_ = normalize_hash(serialized->servers());
    unit_root_ = normalize_hash(serialized->units());
}

bool Tree::Migrate(const StorageDriver& to) const
{
    bool output{true};
    output &= blockchain()->Migrate(to);
    output &= contacts()->Migrate(to);
    output &= credentials()->Migrate(to);
    output &= nyms()->Migrate(to);
    output &= seeds()->Migrate(to);
    output &= servers()->Migrate(to);
    output &= units()->Migrate(to);
    output &= migrate(root_, to);

    return output;
}

Editor<BlockchainTransactions> Tree::mutable_Blockchain()
{
    std::function<void(BlockchainTransactions*, Lock&)> callback =
        [&](BlockchainTransactions* in, Lock& lock) -> void {
        this->save(in, lock);
    };

    return Editor<BlockchainTransactions>(write_lock_, blockchain(), callback);
}

Editor<Contacts> Tree::mutable_Contacts()
{
    std::function<void(Contacts*, Lock&)> callback =
        [&](Contacts* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Contacts>(write_lock_, contacts(), callback);
}

Editor<Credentials> Tree::mutable_Credentials()
{
    std::function<void(Credentials*, Lock&)> callback =
        [&](Credentials* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Credentials>(write_lock_, credentials(), callback);
}

Editor<Nyms> Tree::mutable_Nyms()
{
    std::function<void(Nyms*, Lock&)> callback =
        [&](Nyms* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Nyms>(write_lock_, nyms(), callback);
}

Editor<Seeds> Tree::mutable_Seeds()
{
    std::function<void(Seeds*, Lock&)> callback =
        [&](Seeds* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Seeds>(write_lock_, seeds(), callback);
}

Editor<Servers> Tree::mutable_Servers()
{
    std::function<void(Servers*, Lock&)> callback =
        [&](Servers* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Servers>(write_lock_, servers(), callback);
}

Editor<Units> Tree::mutable_Units()
{
    std::function<void(Units*, Lock&)> callback =
        [&](Units* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Units>(write_lock_, units(), callback);
}

const Nyms& Tree::NymNode() const { return *nyms(); }

Nyms* Tree::nyms() const
{
    Lock lock(nym_lock_);

    if (!nyms_) {
        nyms_.reset(new Nyms(driver_, nym_root_));

        if (!nyms_) {
            otErr << OT_METHOD << __FUNCTION__ << ": Unable to instantiate."
                  << std::endl;
            abort();
        }
    }

    lock.unlock();

    return nyms_.get();
}

bool Tree::save(const Lock& lock) const
{
    if (!verify_write_lock(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) {
        return false;
    }

    return driver_.StoreProto(serialized, root_);
}

void Tree::save(BlockchainTransactions* blockchain, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == blockchain) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    Lock mapLock(blockchain_lock_);
    blockchain_root_ = blockchain->Root();
    mapLock.unlock();

    if (!save(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Tree::save(Contacts* contacts, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == contacts) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    Lock mapLock(contact_lock_);
    contact_root_ = contacts->Root();
    mapLock.unlock();

    if (!save(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Tree::save(Credentials* credentials, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == credentials) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    Lock mapLock(credential_lock_);
    credential_root_ = credentials->Root();
    mapLock.unlock();

    if (!save(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Tree::save(Nyms* nyms, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == nyms) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    Lock mapLock(nym_lock_);
    nym_root_ = nyms->Root();
    mapLock.unlock();

    if (!save(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Tree::save(Seeds* seeds, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == seeds) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    Lock mapLock(seed_lock_);
    seed_root_ = seeds->Root();
    mapLock.unlock();

    if (!save(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Tree::save(Servers* servers, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == servers) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    Lock mapLock(server_lock_);
    server_root_ = servers->Root();
    mapLock.unlock();

    if (!save(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Tree::save(Units* units, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == units) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    Lock mapLock(unit_lock_);
    unit_root_ = units->Root();
    mapLock.unlock();

    if (!save(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

const Seeds& Tree::SeedNode() const { return *seeds(); }

Seeds* Tree::seeds() const
{
    Lock lock(seed_lock_);

    if (!seeds_) {
        seeds_.reset(new Seeds(driver_, seed_root_));

        if (!seeds_) {
            otErr << OT_METHOD << __FUNCTION__ << ": Unable to instantiate."
                  << std::endl;
            abort();
        }
    }

    lock.unlock();

    return seeds_.get();
}

proto::StorageItems Tree::serialize() const
{
    proto::StorageItems serialized;
    serialized.set_version(version_);

    Lock contactLock(contact_lock_);
    serialized.set_contacts(contact_root_);
    contactLock.unlock();

    Lock credLock(credential_lock_);
    serialized.set_creds(credential_root_);
    credLock.unlock();

    Lock nymLock(nym_lock_);
    serialized.set_nyms(nym_root_);
    nymLock.unlock();

    Lock serverLock(server_lock_);
    serialized.set_servers(server_root_);
    serverLock.unlock();

    Lock unitLock(unit_lock_);
    serialized.set_units(unit_root_);
    unitLock.unlock();

    Lock seedLock(seed_lock_);
    serialized.set_seeds(seed_root_);
    seedLock.unlock();

    return serialized;
}

const Servers& Tree::ServerNode() const { return *servers(); }

Servers* Tree::servers() const
{
    Lock lock(server_lock_);

    if (!servers_) {
        servers_.reset(new Servers(driver_, server_root_));

        if (!servers_) {
            otErr << OT_METHOD << __FUNCTION__ << ": Unable to instantiate."
                  << std::endl;
            abort();
        }
    }

    lock.unlock();

    return servers_.get();
}

const Units& Tree::UnitNode() const { return *units(); }

Units* Tree::units() const
{
    Lock lock(unit_lock_);

    if (!units_) {
        units_.reset(new Units(driver_, unit_root_));

        if (!units_) {
            otErr << OT_METHOD << __FUNCTION__ << ": Unable to instantiate."
                  << std::endl;
            abort();
        }
    }

    lock.unlock();

    return units_.get();
}
}  // namespace storage
}  // namespace opentxs
