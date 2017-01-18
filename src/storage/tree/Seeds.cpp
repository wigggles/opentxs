/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anoseedous Numbered Accounts.
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

#include "opentxs/storage/tree/Seeds.hpp"

#include "opentxs/storage/Storage.hpp"

namespace opentxs
{
namespace storage
{
Seeds::Seeds(
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
    }
}

std::string Seeds::Alias(const std::string& id) const { return get_alias(id); }

bool Seeds::check_existing(const std::uint64_t incoming, Metadata& metadata)
{
    const auto& hash = std::get<0>(metadata);
    auto& revision = std::get<2>(metadata);

    // This variable can be zero for two reasons:
    // * The stored version has never been incremented,
    // * The stored version hasn't been loaded yet and so the index
    // hasn't been updated
    // ...so we have to load the seed just to be sure
    if (0 == revision) {
        std::shared_ptr<proto::Seed> existing;

        if (!storage_.LoadProto(hash, existing, false)) {
            std::cerr << __FUNCTION__ << ": Unable to load." << std::endl;
            abort();
        }

        revision = existing->index();
    }

    return (incoming > revision);
}

std::string Seeds::Default() const
{
    std::lock_guard<std::mutex> lock(write_lock_);

    return default_seed_;
}

bool Seeds::Delete(const std::string& id) { return delete_item(id); }

void Seeds::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageSeeds> serialized;
    storage_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load seed index file."
                  << std::endl;
        abort();
    }

    version_ = serialized->version();

    // Fix legacy data stores
    if (0 == version_) {
        version_ = 1;
    }

    default_seed_ = serialized->defaultseed();

    for (const auto& it : serialized->seed()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

bool Seeds::Load(
    const std::string& id,
    std::shared_ptr<proto::Seed>& output,
    std::string& alias,
    const bool checking) const
{
    return load_proto<proto::Seed>(id, output, alias, checking);
}

bool Seeds::save(const std::unique_lock<std::mutex>& lock)
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

proto::StorageSeeds Seeds::serialize() const
{
    proto::StorageSeeds serialized;
    serialized.set_version(version_);
    serialized.set_defaultseed(default_seed_);

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *serialized.add_seed());
        }
    }

    return serialized;
}
bool Seeds::SetAlias(const std::string& id, const std::string& alias)
{
    return set_alias(id, alias);
}

void Seeds::set_default(
    const std::unique_lock<std::mutex>& lock,
    const std::string& id)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    default_seed_ = id;
}

bool Seeds::SetDefault(const std::string& id)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    set_default(lock, id);

    return save(lock);
}

bool Seeds::Store(const proto::Seed& data, const std::string& alias)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    const std::string id = data.fingerprint();
    const auto incomingRevision = data.index();
    const bool existingKey = (item_map_.end() != item_map_.find(id));
    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (existingKey) {
        if (!check_existing(incomingRevision, metadata)) {
            // We're trying to save a seed with a lower index than has already
            // been saved. Just silently skip this update instead.

            return true;
        }
    }

    if (!storage_.StoreProto(data, hash)) {
        return false;
    }

    if (default_seed_.empty()) {
        set_default(lock, id);
    }

    if (!alias.empty()) {
        std::get<1>(metadata) = alias;
    }

    return save(lock);
}
}  // namespace storage
}  // namespace opentxs
