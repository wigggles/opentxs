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

#ifndef OPENTXS_STORAGE_STORAGEDRIVERIMPLEMTNATION_HPP
#define OPENTXS_STORAGE_STORAGEDRIVERIMPLEMTNATION_HPP

#include "opentxs/interface/storage/StoragePlugin.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"

#include <atomic>
#include <string>

namespace opentxs
{
class StorageConfig;

class StoragePlugin_impl : public virtual StoragePlugin
{
public:
    bool EmptyBucket(const bool bucket) const override = 0;

    bool Load(const std::string& key, const bool checking, std::string& value)
        const override;
    bool LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const override = 0;
    bool Store(
        const std::string& key,
        const std::string& value,
        const bool bucket) const override = 0;
    bool Store(const std::string& value, std::string& key) const override;

    bool Migrate(const std::string& key) const override;

    std::string LoadRoot() const override = 0;
    bool StoreRoot(const std::string& hash) const override = 0;

    virtual void Cleanup() = 0;

    virtual ~StoragePlugin_impl() = default;

protected:
    const StorageConfig& config_;
    const Random& random_;

    StoragePlugin_impl(
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        std::atomic<bool>& bucket);
    StoragePlugin_impl() = delete;

private:
    const Digest& digest_;
    std::atomic<bool>& current_bucket_;

    StoragePlugin_impl(const StoragePlugin_impl&) = delete;
    StoragePlugin_impl(StoragePlugin_impl&&) = delete;
    StoragePlugin_impl& operator=(const StoragePlugin_impl&) = delete;
    StoragePlugin_impl& operator=(StoragePlugin_impl&&) = delete;
};

template <class T>
bool StorageDriver::LoadProto(
    const std::string& hash,
    std::shared_ptr<T>& serialized,
    const bool checking) const
{
    std::string raw;
    const bool loaded = Load(hash, checking, raw);
    bool valid = false;

    if (loaded) {
        serialized.reset(new T);
        serialized->ParseFromArray(raw.data(), raw.size());
        valid = proto::Validate<T>(*serialized, VERBOSE);
    }

    if (!valid) {
        if (loaded) {
            std::cerr << "Specified object was located but could not be "
                      << "validated. Database is corrupt." << std::endl
                      << "Hash: " << hash << std::endl
                      << "Size: " << raw.size() << std::endl;
        } else {
            std::cerr << "Specified object is missing. Database is "
                      << "corrupt." << std::endl
                      << "Hash: " << hash << std::endl
                      << "Size: " << raw.size() << std::endl;
        }
    }

    OT_ASSERT(valid);

    return valid;
}

template <class T>
bool StorageDriver::StoreProto(
    const T& data,
    std::string& key,
    std::string& plaintext) const
{
    if (!proto::Validate<T>(data, VERBOSE)) {

        return false;
    }

    plaintext = proto::ProtoAsString<T>(data);

    return Store(plaintext, key);
}

template <class T>
bool StorageDriver::StoreProto(const T& data, std::string& key) const
{
    std::string notUsed;

    return StoreProto<T>(data, key, notUsed);
}

template <class T>
bool StorageDriver::StoreProto(const T& data) const
{
    std::string notUsed;

    return StoreProto<T>(data, notUsed);
}
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_STORAGEDRIVERIMPLEMTNATION_HPP
