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

#ifndef OPENTXS_STORAGE_STORAGE_HPP
#define OPENTXS_STORAGE_STORAGE_HPP

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <opentxs-proto/verify/VerifyCredentials.hpp>
#include <opentxs-proto/verify/VerifyStorage.hpp>

namespace opentxs
{

typedef std::function<bool(const uint32_t, const std::string&, std::string&)>
    Digest;

template<class T>
std::string ProtoAsString(const T& serialized)
{
    int size = serialized.ByteSize();
    char* protoArray = new char [size];

    serialized.SerializeToArray(protoArray, size);
    std::string serializedData(protoArray, size);
    delete[] protoArray;
    return serializedData;
}

// Interface for local storage
class Storage
{
template<class T>
bool LoadProto(
    const std::string hash,
    std::shared_ptr<T>& serialized)
{
    std::string data;

    if (Load(hash, data)) {
        serialized = std::make_shared<T>();
        serialized->ParseFromArray(data.c_str(), data.size());

        return Verify(*serialized);
    }

    return false;
}

template<class T>
bool StoreProto(const T data)
{
    if (nullptr != digest_) {
        std::string plaintext = opentxs::ProtoAsString<T>(data);
        std::string key;
        digest_(Storage::HASH_TYPE, plaintext, key);

        return Store(
            key,
            plaintext);
    }
    return false;
}

private:
    static Storage* instance_pointer_;

    void Read();
    bool UpdateNymCreds(std::string& id, std::string& hash);
    bool UpdateCredentials(std::string& id, std::string& hash);
    bool UpdateNyms(proto::StorageNym& nym);
    bool UpdateItems(const proto::StorageCredentials& creds);
    bool UpdateItems(const proto::StorageNymList& nyms);
    bool UpdateRoot(const proto::StorageItems& items);

    Storage(Storage const&) = delete;
    Storage& operator=(Storage const&) = delete;

protected:
    std::string root_ = "";
    std::string items_ = "";
    const uint32_t HASH_TYPE = 2; // BTC160
    Digest digest_ = nullptr;
    std::map<std::string, std::string> credentials_{{}};
    std::map<std::string, std::string> nyms_{{}};
    bool isLoaded_ = false;

    Storage(const Digest& hash);
    virtual void Init(const Digest& hash);

    virtual std::string LoadRoot() = 0;
    virtual bool Load(const std::string& key, std::string& value) = 0;
    virtual bool StoreRoot(const std::string& hash) = 0;
    virtual bool Store(const std::string& key, const std::string& value) = 0;

public:
    enum class Type : std::uint8_t {
        ERROR = 0,
        FS = 1
    };

    static Storage& Factory(
        const Digest& hash,
        const std::string& param = "",
        Type type = Type::ERROR);

    bool Load(
        const std::string id,
        std::shared_ptr<proto::Credential>& cred);
    bool Load(
        const std::string id,
        std::shared_ptr<proto::CredentialIndex>& cred);
    bool Store(const proto::Credential& data);
    bool Store(const proto::CredentialIndex& data);

    virtual void Cleanup();
    virtual ~Storage();
};

}  // namespace opentxs
#endif // OPENTXS_STORAGE_STORAGE_HPP
