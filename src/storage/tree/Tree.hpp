// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "storage/tree/Node.hpp"

namespace opentxs
{
namespace api
{
namespace implementation
{
class Storage;
}  // namespace implementation

namespace storage
{
class Driver;
}  // namespace storage
}  // namespace api

namespace proto
{
class Ciphertext;
}  // namespace proto

namespace storage
{
class Accounts;
class BlockchainTransactions;
class Contacts;
class Credentials;
class Notary;
class Nyms;
class Seeds;
class Servers;
class Units;
}  // namespace storage
}  // namespace opentxs

namespace opentxs::storage
{
class Tree final : public Node
{
public:
    const storage::Accounts& Accounts() const;
    const storage::BlockchainTransactions& Blockchain() const;
    const storage::Contacts& Contacts() const;
    const storage::Credentials& Credentials() const;
    const storage::Notary& Notary(const std::string& id) const;
    const storage::Nyms& Nyms() const;
    const storage::Seeds& Seeds() const;
    const storage::Servers& Servers() const;
    const storage::Units& Units() const;

    Editor<storage::Accounts> mutable_Accounts();
    Editor<storage::BlockchainTransactions> mutable_Blockchain();
    Editor<storage::Contacts> mutable_Contacts();
    Editor<storage::Credentials> mutable_Credentials();
    Editor<storage::Notary> mutable_Notary(const std::string& id);
    Editor<storage::Nyms> mutable_Nyms();
    Editor<storage::Seeds> mutable_Seeds();
    Editor<storage::Servers> mutable_Servers();
    Editor<storage::Units> mutable_Units();

    bool Load(
        std::shared_ptr<proto::Ciphertext>& output,
        const bool checking = false) const;
    bool Migrate(const opentxs::api::storage::Driver& to) const final;

    bool Store(const proto::Ciphertext& serialized);

    ~Tree() final;

private:
    friend class api::implementation::Storage;
    friend class Root;

    std::string account_root_{Node::BLANK_HASH};
    std::string blockchain_root_{Node::BLANK_HASH};
    std::string contact_root_{Node::BLANK_HASH};
    std::string credential_root_{Node::BLANK_HASH};
    std::string notary_root_{Node::BLANK_HASH};
    std::string nym_root_{Node::BLANK_HASH};
    std::string seed_root_{Node::BLANK_HASH};
    std::string server_root_{Node::BLANK_HASH};
    std::string unit_root_{Node::BLANK_HASH};

    mutable std::mutex account_lock_;
    mutable std::unique_ptr<storage::Accounts> account_;
    mutable std::mutex blockchain_lock_;
    mutable std::unique_ptr<storage::BlockchainTransactions> blockchain_;
    mutable std::mutex contact_lock_;
    mutable std::unique_ptr<storage::Contacts> contacts_;
    mutable std::mutex credential_lock_;
    mutable std::unique_ptr<storage::Credentials> credentials_;
    mutable std::mutex notary_lock_;
    mutable std::unique_ptr<storage::Notary> notary_;
    mutable std::mutex nym_lock_;
    mutable std::unique_ptr<storage::Nyms> nyms_;
    mutable std::mutex seed_lock_;
    mutable std::unique_ptr<storage::Seeds> seeds_;
    mutable std::mutex server_lock_;
    mutable std::unique_ptr<storage::Servers> servers_;
    mutable std::mutex unit_lock_;
    mutable std::unique_ptr<storage::Units> units_;
    mutable std::mutex master_key_lock_;
    mutable std::shared_ptr<proto::Ciphertext> master_key_;

    template <typename T, typename... Args>
    T* get_child(
        std::mutex& mutex,
        std::unique_ptr<T>& pointer,
        const std::string& hash,
        Args&&... params) const;
    template <typename T, typename... Args>
    Editor<T> get_editor(
        std::mutex& mutex,
        std::unique_ptr<T>& pointer,
        std::string& hash,
        Args&&... params) const;
    storage::Accounts* accounts() const;
    storage::BlockchainTransactions* blockchain() const;
    storage::Contacts* contacts() const;
    storage::Credentials* credentials() const;
    storage::Notary* notary(const std::string& id) const;
    storage::Nyms* nyms() const;
    storage::Seeds* seeds() const;
    storage::Servers* servers() const;
    storage::Units* units() const;

    void init(const std::string& hash) final;
    bool save(const Lock& lock) const final;
    template <typename T>
    void save_child(
        T*,
        const Lock& lock,
        std::mutex& hashLock,
        std::string& hash) const;
    proto::StorageItems serialize() const;
    bool update_root(const std::string& hash);

    Tree(const opentxs::api::storage::Driver& storage, const std::string& key);
    Tree() = delete;
    Tree(const Tree&);
    Tree(Tree&&) = delete;
    Tree operator=(const Tree&) = delete;
    Tree operator=(Tree&&) = delete;
};
}  // namespace opentxs::storage
