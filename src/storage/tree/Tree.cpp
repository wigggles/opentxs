// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Tree.hpp"

#include "storage/Plugin.hpp"
#include "Accounts.hpp"
#include "BlockchainTransactions.hpp"
#include "Contacts.hpp"
#include "Credentials.hpp"
#include "Notary.hpp"
#include "Nym.hpp"
#include "Nyms.hpp"
#include "Seeds.hpp"
#include "Servers.hpp"
#include "Units.hpp"

#define TREE_VERSION 5

#define OT_METHOD "opentxs::storage::Tree::"

namespace opentxs::storage
{
Tree::Tree(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
    , account_root_(Node::BLANK_HASH)
    , blockchain_root_(Node::BLANK_HASH)
    , contact_root_(Node::BLANK_HASH)
    , credential_root_(Node::BLANK_HASH)
    , nym_root_(Node::BLANK_HASH)
    , seed_root_(Node::BLANK_HASH)
    , server_root_(Node::BLANK_HASH)
    , unit_root_(Node::BLANK_HASH)
    , account_lock_()
    , account_(nullptr)
    , blockchain_lock_()
    , blockchain_(nullptr)
    , contact_lock_()
    , contacts_(nullptr)
    , credential_lock_()
    , credentials_(nullptr)
    , notary_lock_()
    , notary_(nullptr)
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
        version_ = TREE_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

Tree::Tree(const Tree& rhs)
    : Node(rhs.driver_, "")
    , account_root_(Node::BLANK_HASH)
    , blockchain_root_(Node::BLANK_HASH)
    , contact_root_(Node::BLANK_HASH)
    , credential_root_(Node::BLANK_HASH)
    , nym_root_(Node::BLANK_HASH)
    , seed_root_(Node::BLANK_HASH)
    , server_root_(Node::BLANK_HASH)
    , unit_root_(Node::BLANK_HASH)
    , account_lock_()
    , account_(nullptr)
    , blockchain_lock_()
    , blockchain_(nullptr)
    , contact_lock_()
    , contacts_(nullptr)
    , credential_lock_()
    , credentials_(nullptr)
    , notary_lock_()
    , notary_(nullptr)
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
    account_root_ = rhs.account_root_;
    blockchain_root_ = rhs.blockchain_root_;
    contact_root_ = rhs.contact_root_;
    credential_root_ = rhs.credential_root_;
    notary_root_ = rhs.notary_root_;
    nym_root_ = rhs.nym_root_;
    seed_root_ = rhs.seed_root_;
    server_root_ = rhs.server_root_;
    unit_root_ = rhs.unit_root_;
}

storage::Accounts* Tree::accounts() const
{
    return get_child<storage::Accounts>(account_lock_, account_, account_root_);
}

storage::BlockchainTransactions* Tree::blockchain() const
{
    return get_child<storage::BlockchainTransactions>(
        blockchain_lock_, blockchain_, blockchain_root_);
}

const storage::Accounts& Tree::Accounts() const { return *accounts(); }

const storage::BlockchainTransactions& Tree::Blockchain() const
{
    return *blockchain();
}

const storage::Contacts& Tree::Contacts() const { return *contacts(); }

storage::Contacts* Tree::contacts() const
{
    return get_child<storage::Contacts>(
        contact_lock_, contacts_, contact_root_);
}

const storage::Credentials& Tree::Credentials() const { return *credentials(); }

storage::Credentials* Tree::credentials() const
{
    return get_child<storage::Credentials>(
        credential_lock_, credentials_, credential_root_);
}

template <typename T, typename... Args>
T* Tree::get_child(
    std::mutex& mutex,
    std::unique_ptr<T>& pointer,
    const std::string& hash,
    Args&&... params) const
{
    Lock lock(mutex);

    if (false == bool(pointer)) {
        pointer.reset(new T(driver_, hash, params...));

        if (false == bool(pointer)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate.")
                .Flush();

            OT_FAIL;
        }
    }

    lock.unlock();

    return pointer.get();
}

template <typename T, typename... Args>
Editor<T> Tree::get_editor(
    std::mutex& mutex,
    std::unique_ptr<T>& pointer,
    std::string& hash,
    Args&&... params) const
{
    std::function<void(T*, Lock&)> callback = [&](T* in, Lock& lock) -> void {
        save_child<T>(in, lock, mutex, hash);
    };

    return Editor<T>(
        write_lock_, get_child<T>(mutex, pointer, hash, params...), callback);
}

void Tree::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageItems> serialized{nullptr};
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load root index file.")
            .Flush();

        OT_FAIL
    }

    version_ = serialized->version();

    // Upgrade version
    if (TREE_VERSION > version_) { version_ = TREE_VERSION; }

    account_root_ = normalize_hash(serialized->accounts());
    blockchain_root_ = normalize_hash(serialized->blockchaintransactions());
    contact_root_ = normalize_hash(serialized->contacts());
    credential_root_ = normalize_hash(serialized->creds());
    notary_root_ = normalize_hash(serialized->notary());
    nym_root_ = normalize_hash(serialized->nyms());
    seed_root_ = normalize_hash(serialized->seeds());
    server_root_ = normalize_hash(serialized->servers());
    unit_root_ = normalize_hash(serialized->units());
}

bool Tree::Migrate(const opentxs::api::storage::Driver& to) const
{
    bool output{true};
    output &= accounts()->Migrate(to);
    output &= blockchain()->Migrate(to);
    output &= contacts()->Migrate(to);
    output &= credentials()->Migrate(to);
    output &= notary("")->Migrate(to);
    output &= nyms()->Migrate(to);
    output &= seeds()->Migrate(to);
    output &= servers()->Migrate(to);
    output &= units()->Migrate(to);
    output &= migrate(root_, to);

    return output;
}

Editor<storage::Accounts> Tree::mutable_Accounts()
{
    return get_editor<storage::Accounts>(
        account_lock_, account_, account_root_);
}

Editor<storage::BlockchainTransactions> Tree::mutable_Blockchain()
{
    return get_editor<storage::BlockchainTransactions>(
        blockchain_lock_, blockchain_, blockchain_root_);
}

Editor<storage::Contacts> Tree::mutable_Contacts()
{
    return get_editor<storage::Contacts>(
        contact_lock_, contacts_, contact_root_);
}

Editor<storage::Credentials> Tree::mutable_Credentials()
{
    return get_editor<storage::Credentials>(
        credential_lock_, credentials_, credential_root_);
}

Editor<storage::Notary> Tree::mutable_Notary(const std::string& id)
{
    return get_editor<storage::Notary>(notary_lock_, notary_, notary_root_, id);
}

Editor<storage::Nyms> Tree::mutable_Nyms()
{
    return get_editor<storage::Nyms>(nym_lock_, nyms_, nym_root_);
}

Editor<storage::Seeds> Tree::mutable_Seeds()
{
    return get_editor<storage::Seeds>(seed_lock_, seeds_, seed_root_);
}

Editor<storage::Servers> Tree::mutable_Servers()
{
    return get_editor<storage::Servers>(server_lock_, servers_, server_root_);
}

Editor<storage::Units> Tree::mutable_Units()
{
    return get_editor<storage::Units>(unit_lock_, units_, unit_root_);
}

const storage::Notary& Tree::Notary(const std::string& id) const
{
    return *notary(id);
}

const storage::Nyms& Tree::Nyms() const { return *nyms(); }

storage::Notary* Tree::notary(const std::string& id) const
{
    return get_child<storage::Notary>(notary_lock_, notary_, notary_root_, id);
}

storage::Nyms* Tree::nyms() const
{
    return get_child<storage::Nyms>(nym_lock_, nyms_, nym_root_);
}

bool Tree::save(const Lock& lock) const
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

template <typename T>
void Tree::save_child(
    T* input,
    const Lock& lock,
    std::mutex& hashLock,
    std::string& hash) const
{
    if (false == verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    if (nullptr == input) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null target.").Flush();
        OT_FAIL
    }

    Lock rootLock(hashLock);
    hash = input->Root();
    rootLock.unlock();

    if (false == save(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Save error.").Flush();
        OT_FAIL
    }
}

const storage::Seeds& Tree::Seeds() const { return *seeds(); }

storage::Seeds* Tree::seeds() const
{
    return get_child<storage::Seeds>(seed_lock_, seeds_, seed_root_);
}

proto::StorageItems Tree::serialize() const
{
    proto::StorageItems serialized;
    serialized.set_version(version_);

    Lock accountLock(account_lock_);
    serialized.set_accounts(account_root_);
    accountLock.unlock();

    Lock blockchainLock(blockchain_lock_);
    serialized.set_blockchaintransactions(blockchain_root_);
    blockchainLock.unlock();

    Lock contactLock(contact_lock_);
    serialized.set_contacts(contact_root_);
    contactLock.unlock();

    Lock credLock(credential_lock_);
    serialized.set_creds(credential_root_);
    credLock.unlock();

    Lock notaryLock(notary_lock_);
    serialized.set_notary(notary_root_);
    notaryLock.unlock();

    Lock nymLock(nym_lock_);
    serialized.set_nyms(nym_root_);
    nymLock.unlock();

    Lock seedLock(seed_lock_);
    serialized.set_seeds(seed_root_);
    seedLock.unlock();

    Lock serverLock(server_lock_);
    serialized.set_servers(server_root_);
    serverLock.unlock();

    Lock unitLock(unit_lock_);
    serialized.set_units(unit_root_);
    unitLock.unlock();

    return serialized;
}

const storage::Servers& Tree::Servers() const { return *servers(); }

storage::Servers* Tree::servers() const
{
    return get_child<storage::Servers>(server_lock_, servers_, server_root_);
}

const storage::Units& Tree::Units() const { return *units(); }

storage::Units* Tree::units() const
{
    return get_child<storage::Units>(unit_lock_, units_, unit_root_);
}

Tree::~Tree() = default;
}  // namespace opentxs::storage
