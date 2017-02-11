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

#include "opentxs/storage/tree/Tree.hpp"

#include "opentxs/storage/tree/Credentials.hpp"
#include "opentxs/storage/tree/Nym.hpp"
#include "opentxs/storage/tree/Nyms.hpp"
#include "opentxs/storage/tree/Seeds.hpp"
#include "opentxs/storage/tree/Servers.hpp"
#include "opentxs/storage/tree/Units.hpp"
#include "opentxs/storage/Storage.hpp"

namespace opentxs
{
namespace storage
{

Tree::Tree(
    const Storage& storage,
    const keyFunction& migrate,
    const std::string& hash)
    : Node(storage, migrate, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = 1;
        root_ = Node::BLANK_HASH;
        credential_root_ = Node::BLANK_HASH;
        nym_root_ = Node::BLANK_HASH;
        seed_root_ = Node::BLANK_HASH;
        server_root_ = Node::BLANK_HASH;
        unit_root_ = Node::BLANK_HASH;
    }
}

Tree::Tree(const Tree& rhs)
    : Node(rhs.storage_, rhs.migrate_, "")
{
    std::lock_guard<std::mutex> lock(rhs.write_lock_);

    version_ = rhs.version_;
    root_ = rhs.root_;
    credential_root_ = rhs.credential_root_;
    nym_root_ = rhs.nym_root_;
    seed_root_ = rhs.seed_root_;
    server_root_ = rhs.server_root_;
    unit_root_ = rhs.unit_root_;
}

const Credentials& Tree::CredentialNode() { return *credentials(); }

Credentials* Tree::credentials() const
{
    std::unique_lock<std::mutex> lock(credential_lock_);

    if (!credentials_) {
        credentials_.reset(
            new Credentials(storage_, migrate_, credential_root_));

        if (!credentials_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return credentials_.get();
}

void Tree::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageItems> serialized;
    storage_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load root index file."
                  << std::endl;
        abort();
    }

    version_ = serialized->version();

    // Upgrade to version 1
    if (1 > version_) {
        version_ = 1;
    }

    credential_root_ = serialized->creds();
    nym_root_ = serialized->nyms();
    server_root_ = serialized->servers();
    unit_root_ = serialized->units();
    seed_root_ = serialized->seeds();
}

bool Tree::Migrate() const
{
    if (!credentials()->Migrate()) {
        return false;
    }

    if (!nyms()->Migrate()) {
        return false;
    }

    if (!seeds()->Migrate()) {
        return false;
    }

    if (!servers()->Migrate()) {
        return false;
    }

    if (!units()->Migrate()) {
        return false;
    }

    return migrate(root_);
}

Editor<Credentials> Tree::mutable_Credentials()
{
    std::function<void(Credentials*, std::unique_lock<std::mutex>&)> callback =
        [&](Credentials* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock);
    };

    return Editor<Credentials>(write_lock_, credentials(), callback);
}

Editor<Nyms> Tree::mutable_Nyms()
{
    std::function<void(Nyms*, std::unique_lock<std::mutex>&)> callback =
        [&](Nyms* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock);
    };

    return Editor<Nyms>(write_lock_, nyms(), callback);
}

Editor<Seeds> Tree::mutable_Seeds()
{
    std::function<void(Seeds*, std::unique_lock<std::mutex>&)> callback =
        [&](Seeds* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock);
    };

    return Editor<Seeds>(write_lock_, seeds(), callback);
}

Editor<Servers> Tree::mutable_Servers()
{
    std::function<void(Servers*, std::unique_lock<std::mutex>&)> callback =
        [&](Servers* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock);
    };

    return Editor<Servers>(write_lock_, servers(), callback);
}

Editor<Units> Tree::mutable_Units()
{
    std::function<void(Units*, std::unique_lock<std::mutex>&)> callback =
        [&](Units* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock);
    };

    return Editor<Units>(write_lock_, units(), callback);
}

const Nyms& Tree::NymNode() { return *nyms(); }

Nyms* Tree::nyms() const
{
    std::unique_lock<std::mutex> lock(nym_lock_);

    if (!nyms_) {
        nyms_.reset(new Nyms(storage_, migrate_, nym_root_));

        if (!nyms_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return nyms_.get();
}

bool Tree::save(const std::unique_lock<std::mutex>& lock)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Check(serialized, version_, version_)) {
        return false;
    }

    return storage_.StoreProto(serialized, root_);
}

void Tree::save(
    Credentials* credentials,
    const std::unique_lock<std::mutex>& lock)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == credentials) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    std::unique_lock<std::mutex> mapLock(credential_lock_);
    credential_root_ = credentials->Root();
    mapLock.unlock();

    if (!save(lock)) {
        std::cerr << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Tree::save(Nyms* nyms, const std::unique_lock<std::mutex>& lock)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == nyms) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    std::unique_lock<std::mutex> mapLock(nym_lock_);
    nym_root_ = nyms->Root();
    mapLock.unlock();

    if (!save(lock)) {
        std::cerr << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Tree::save(Seeds* seeds, const std::unique_lock<std::mutex>& lock)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == seeds) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    std::unique_lock<std::mutex> mapLock(seed_lock_);
    seed_root_ = seeds->Root();
    mapLock.unlock();

    if (!save(lock)) {
        std::cerr << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Tree::save(Servers* servers, const std::unique_lock<std::mutex>& lock)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == servers) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    std::unique_lock<std::mutex> mapLock(server_lock_);
    server_root_ = servers->Root();
    mapLock.unlock();

    if (!save(lock)) {
        std::cerr << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Tree::save(Units* units, const std::unique_lock<std::mutex>& lock)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == units) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    std::unique_lock<std::mutex> mapLock(unit_lock_);
    unit_root_ = units->Root();
    mapLock.unlock();

    if (!save(lock)) {
        std::cerr << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

const Seeds& Tree::SeedNode() { return *seeds(); }

Seeds* Tree::seeds() const
{
    std::unique_lock<std::mutex> lock(seed_lock_);

    if (!seeds_) {
        seeds_.reset(new Seeds(storage_, migrate_, seed_root_));

        if (!seeds_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
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

    std::unique_lock<std::mutex> credLock(credential_lock_);
    serialized.set_creds(credential_root_);
    credLock.unlock();

    std::unique_lock<std::mutex> nymLock(nym_lock_);
    serialized.set_nyms(nym_root_);
    nymLock.unlock();

    std::unique_lock<std::mutex> serverLock(server_lock_);
    serialized.set_servers(server_root_);
    serverLock.unlock();

    std::unique_lock<std::mutex> unitLock(unit_lock_);
    serialized.set_units(unit_root_);
    unitLock.unlock();

    std::unique_lock<std::mutex> seedLock(seed_lock_);
    serialized.set_seeds(seed_root_);
    seedLock.unlock();

    return serialized;
}

const Servers& Tree::ServerNode() { return *servers(); }

Servers* Tree::servers() const
{
    std::unique_lock<std::mutex> lock(server_lock_);

    if (!servers_) {
        servers_.reset(new Servers(storage_, migrate_, server_root_));

        if (!servers_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return servers_.get();
}

const Units& Tree::UnitNode() { return *units(); }

Units* Tree::units() const
{
    std::unique_lock<std::mutex> lock(unit_lock_);

    if (!units_) {
        units_.reset(new Units(storage_, migrate_, unit_root_));

        if (!units_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return units_.get();
}
}  // namespace storage
}  // namespace opentxs
