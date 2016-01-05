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

Storage::Storage(const Digest& hash)
{
    Init(hash);
}

void Storage::Init(const Digest& hash)
{
    digest_ = hash;
}

Storage& Storage::Factory(const Digest& hash, const std::string& param, Type type)
{
    if (nullptr == instance_pointer_) {
        switch (type) {
            case Type::ERROR :
                std::cout
                << "Warning: replacing bad type with default." << std::endl;

                //intentional fall-through
            default :
                instance_pointer_ = new StorageFS(param, hash);
        }
    }

    assert(nullptr != instance_pointer_);

    return *instance_pointer_;
}

void Storage::Read()
{
    isLoaded_ = true;

    std::string rootHash = LoadRoot();

    if (rootHash.empty()) { return; }

    std::shared_ptr<proto::StorageRoot> root;

    if (!LoadProto<proto::StorageRoot>(rootHash, root)) { return; }

    std::shared_ptr<proto::StorageItems> items;

    if (!LoadProto<proto::StorageItems>(root->items(), items)) { return; }

    std::shared_ptr<proto::StorageCredentials> creds;

    if (!LoadProto<proto::StorageCredentials>(items->creds(), creds)) { return; }

    for (auto& it : creds->cred()) {
        credentials_.insert(std::pair<std::string, std::string>(it.itemid(), it.hash()));
    }
}

bool Storage::Store(const proto::Credential& data)
{
    if (!isLoaded_) { Read(); }

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
            if (!cred.first.empty() && !cred.first.empty()) {
                proto::StorageItemHash* item = credIndex.add_cred();
                item->set_version(1);
                item->set_itemid(cred.first);
                item->set_hash(cred.second);
            }
        }

        assert(Verify(credIndex));

        bool savedIndex = StoreProto<proto::StorageCredentials>(credIndex);

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

        assert(Verify(items));

        bool savedItems = StoreProto<proto::StorageItems>(items);

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

        assert(Verify(root));

        bool savedRoot = StoreProto<proto::StorageRoot>(root);

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
    if (!isLoaded_) { Read(); }

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
