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

#ifndef OPENTXS_STORAGE_TREE_NODE_HPP
#define OPENTXS_STORAGE_TREE_NODE_HPP

#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/storage/Storage.hpp"

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <tuple>

namespace opentxs
{

namespace storage
{

typedef std::function<bool(const std::string&)> keyFunction;
/** A set of metadata associated with a stored object
    *  * string: hash
    *  * string: alias
    *  * uint64: revision
    *  * bool:   private
    */
typedef std::tuple<std::string, std::string, std::uint64_t, bool> Metadata;
/** Maps a logical id to the stored metadata for the object
    *  * string: id of the stored object
    *  * Metadata: metadata for the stored object
    */
typedef std::map<std::string, Metadata> Index;

class Node
{
protected:
    template <class T>
    bool store_proto(
        const T& data,
        const std::string& id,
        const std::string& alias,
        std::string& plaintext)
    {
        std::unique_lock<std::mutex> lock(write_lock_);

        auto& metadata = item_map_[id];
        auto& hash = std::get<0>(metadata);

        if (!storage_.StoreProto<T>(data, hash, plaintext)) {
            return false;
        }

        if (!alias.empty()) {
            std::get<1>(metadata) = alias;
        }

        return save(lock);
    }

    template <class T>
    bool store_proto(
        const T& data,
        const std::string& id,
        const std::string& alias)
    {
        std::string notUsed;

        return store_proto<T>(data, id, alias, notUsed);
    }

    template <class T>
    bool load_proto(
        const std::string& id,
        std::shared_ptr<T>& output,
        std::string& alias,
        const bool checking) const
    {
        std::lock_guard<std::mutex> lock(write_lock_);
        const auto& it = item_map_.find(id);
        const bool exists = (item_map_.end() != it);

        if (!exists) {
            if (!checking) {
                std::cout << __FUNCTION__ << ": Error: item with id " << id
                          << " does not exist." << std::endl;
            }

            return false;
        }

        alias = std::get<1>(it->second);

        return storage_.LoadProto<T>(std::get<0>(it->second), output, checking);
    }

    template <class T>
    void map(const std::function<void(const T&)> input) const
    {
        std::unique_lock<std::mutex> lock(write_lock_);
        const auto copy = item_map_;
        write_lock_.unlock();

        for (const auto& it : copy) {
            std::shared_ptr<T> serialized;

            if (storage_.LoadProto<T>(
                    std::get<0>(it.second), serialized, false)) {
                input(*serialized);
            }
        }
    }

private:
    Node() = delete;
    Node(const Node&) = delete;
    Node(Node&&) = delete;
    Node& operator=(const Node&) = delete;
    Node& operator=(Node&&) = delete;

protected:
    static const std::string BLANK_HASH;

    const Storage& storage_;
    const keyFunction& migrate_;

    std::uint32_t version_{0};
    std::string root_;

    mutable std::mutex write_lock_;
    mutable Index item_map_;

    bool check_hash(const std::string& hash) const;
    std::string get_alias(const std::string& id) const;
    bool load_raw(
        const std::string& id,
        std::string& output,
        std::string& alias,
        const bool checking) const;
    bool migrate(const std::string& hash) const;
    void serialize_index(
        const std::string& id,
        const Metadata& metadata,
        proto::StorageItemHash& output,
        const proto::StorageHashType type = proto::STORAGEHASH_PROTO) const;

    bool delete_item(const std::string& id);
    bool set_alias(const std::string& id, const std::string& alias);
    void set_hash(
        const std::uint32_t version,
        const std::string& id,
        const std::string& hash,
        proto::StorageItemHash& output,
        const proto::StorageHashType type = proto::STORAGEHASH_PROTO) const;
    virtual bool save(const std::unique_lock<std::mutex>& lock) = 0;
    bool store_raw(
        const std::string& data,
        const std::string& id,
        const std::string& alias);
    bool verify_write_lock(const std::unique_lock<std::mutex>& lock) const;

    virtual void init(const std::string& hash) = 0;

    Node(
        const Storage& storage,
        const keyFunction& migrate,
        const std::string& key);

public:
    ObjectList List() const;
    virtual bool Migrate() const;
    std::string Root() const;

    virtual ~Node() = default;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_NODE_HPP
