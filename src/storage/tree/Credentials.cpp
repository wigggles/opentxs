// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Credentials.hpp"

#include "storage/Plugin.hpp"

namespace opentxs
{
namespace storage
{
Credentials::Credentials(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = 2;
        root_ = Node::BLANK_HASH;
    }
}

std::string Credentials::Alias(const std::string& id) const
{
    return get_alias(id);
}

bool Credentials::check_existing(const bool incoming, Metadata& metadata) const
{
    const auto& hash = std::get<0>(metadata);
    auto& isPrivate = std::get<3>(metadata);

    if (incoming) {
        // If the credential to be saved is private, we're going to save it
        // regardless of the state of the existing version.
        isPrivate = true;

        return isPrivate;
    }

    // This variable can be false for two reasons:
    // * The stored version is public,
    // * It's private but hasn't been loaded yet and so the index
    // hasn't been updated
    // ...so we have to load the credential just to be sure
    if (!isPrivate) {
        std::shared_ptr<proto::Credential> existing;

        if (!driver_.LoadProto(hash, existing, false)) {
            std::cerr << __FUNCTION__ << ": Failed to load object" << std::endl;
            abort();
        }

        isPrivate = (proto::KEYMODE_PRIVATE == existing->mode());
    }

    return !isPrivate;
}

bool Credentials::Delete(const std::string& id) { return delete_item(id); }

void Credentials::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageCredentials> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load credentials index file."
                  << std::endl;
        abort();
    }

    version_ = serialized->version();

    // Upgrade to version 2
    if (2 > version_) { version_ = 2; }

    for (const auto& it : serialized->cred()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

bool Credentials::Load(
    const std::string& id,
    std::shared_ptr<proto::Credential>& cred,
    const bool checking) const
{
    std::lock_guard<std::mutex> lock(write_lock_);
    const bool exists = (item_map_.end() != item_map_.find(id));

    if (!exists) {
        if (!checking) {
            std::cerr << __FUNCTION__ << ": Error: credential with id " << id
                      << " does not exist." << std::endl;
        }

        return false;
    }

    auto& metadata = item_map_[id];
    const auto& hash = std::get<0>(metadata);
    auto& isPrivate = std::get<3>(metadata);
    const bool loaded = driver_.LoadProto(hash, cred, checking);

    if (!loaded) { return false; }

    isPrivate = (proto::KEYMODE_PRIVATE == cred->mode());

    return true;
}

bool Credentials::save(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

proto::StorageCredentials Credentials::serialize() const
{
    proto::StorageCredentials serialized;
    serialized.set_version(version_);

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_cred());
        }
    }

    return serialized;
}

bool Credentials::SetAlias(const std::string& id, const std::string& alias)
{
    return set_alias(id, alias);
}

bool Credentials::Store(const proto::Credential& cred, const std::string& alias)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    const std::string id = cred.id();
    const bool existingKey = (item_map_.end() != item_map_.find(id));
    const bool incomingPrivate = (proto::KEYMODE_PRIVATE == cred.mode());
    const bool incomingPublic = (proto::KEYMODE_PUBLIC == cred.mode());

    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (existingKey && incomingPublic) {
        if (!check_existing(incomingPrivate, metadata)) {
            // We're trying to save a public credential but already have
            // the private version saved. Just silently skip this update
            // instead of overwriting private keys.

            return true;
        }
    }

    if (!driver_.StoreProto(cred, hash)) { return false; }

    if (!alias.empty()) { std::get<1>(metadata) = alias; }

    return save(lock);
}
}  // namespace storage
}  // namespace opentxs
