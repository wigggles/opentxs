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

#include <opentxs/storage/Storage.hpp>

#include <iostream>

#include <opentxs/storage/StorageFS.hpp>

namespace opentxs
{
Storage* Storage::instance_pointer_ = nullptr;

Storage::Storage(Digest& hash)
{
    Init(hash);
}

void Storage::Init(Digest& hash)
{
    digest_ = hash;
}

Storage& Storage::Factory(Digest& hash, std::string param, Type type)
{
    if (nullptr == instance_pointer_)
    {
        switch (type) {
            case Type::ERROR :
                std::cout
                << "Warning: replacing bad type with default." << std::endl;

                //intentional fall-through
            default :
                instance_pointer_ = new StorageFS(param, hash);
        }
    }

    return *instance_pointer_;
}

bool Storage::Store(const proto::StorageCredentials& data)
{
    if (nullptr != digest_) {
        std::string plaintext = ProtoAsString<proto::StorageCredentials>(data);
        std::string key;
        digest_(Storage::HASH_TYPE, plaintext, key);

        return Store(
            key,
            plaintext);
    }
    return false;
}

bool Storage::Store(const proto::StorageItems& data)
{
    if (nullptr != digest_) {
        std::string plaintext = ProtoAsString<proto::StorageItems>(data);
        std::string key;
        digest_(Storage::HASH_TYPE, plaintext, key);

        return Store(
            key,
            plaintext);
    }
    return false;
}

bool Storage::Store(const proto::StorageRoot& data)
{
    if (nullptr != digest_) {
        std::string plaintext = ProtoAsString<proto::StorageRoot>(data);
        std::string key;
        digest_(Storage::HASH_TYPE, plaintext, key);

        return Store(
            key,
            plaintext);
    }
    return false;
}

bool Storage::Store(const proto::Credential& data)
{
    if (nullptr != digest_) {
        std::string plaintext = ProtoAsString<proto::Credential>(data);
        std::string key;
        digest_(Storage::HASH_TYPE, plaintext, key);

        bool savedCredential = Store(
            key,
            plaintext);

        if (savedCredential) {
            std::string credID = data.id();
            return UpdateCredentials(credID, key);
        }
    }
    return false;
}

bool Storage::UpdateCredentials(std::string id, std::string hash)
{
    if ((!id.empty()) && (!hash.empty())) {
        credentials_.insert(std::pair<std::string, std::string>(id, hash));

        proto::StorageCredentials credIndex;
        credIndex.set_version(1);
        for (auto& cred : credentials_) {
            proto::StorageItemHash* item = credIndex.add_cred();
            item->set_version(1);
            item->set_itemid(cred.first);
            item->set_hash(cred.second);
        }

        bool savedIndex = Store(credIndex);

        if (savedIndex) {
            return UpdateItems(credIndex);
        }
    }

    return false;
}

bool Storage::UpdateItems(const proto::StorageCredentials& creds)
{
    if (nullptr != digest_) {
        std::string plaintext = ProtoAsString<proto::StorageCredentials>(creds);
        std::string hash;
        digest_(Storage::HASH_TYPE, plaintext, hash);

        proto::StorageItems items;
        items.set_version(1);
        items.set_creds(hash);

        bool savedItems = Store(items);

        if (savedItems) {
            return UpdateRoot(items);
        }
    }

    return false;
}

bool Storage::UpdateRoot(const proto::StorageItems& items)
{
    if (nullptr != digest_) {
        std::string plaintext = ProtoAsString<proto::StorageItems>(items);
        std::string hash;
        digest_(Storage::HASH_TYPE, plaintext, hash);

        proto::StorageRoot root;
        root.set_version(1);
        root.set_items(hash);

        bool savedRoot = Store(root);

        if (savedRoot) {
            plaintext = ProtoAsString<proto::StorageRoot>(root);
            digest_(Storage::HASH_TYPE, plaintext, hash);

            root_ = hash;

            return StoreRoot(hash);
        }
    }

    return false;
}

bool Storage::Load(
    const std::string id,
    std::shared_ptr<proto::Credential>& cred)
{
    auto it = credentials_.find(id);

    if (it != credentials_.end()) {
        std::string data;
        bool loaded = Load(it->second, data);

        if (loaded) {
            cred = std::make_shared<proto::Credential>();
            cred->ParseFromArray(data.c_str(), data.size());

            return Verify(*cred);
        }

    }

    return false;
}

void Storage::Cleanup()
{
}

Storage::~Storage()
{
    Cleanup();
}


} // namespace opentxs
