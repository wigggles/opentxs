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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/storage/StoragePlugin.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/storage/Storage.hpp"

namespace opentxs
{

StoragePlugin_impl::StoragePlugin_impl(
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    std::atomic<bool>& bucket)
    : config_(config)
    , random_(random)
    , digest_(hash)
    , current_bucket_(bucket)
{
    // There's a bootstrapping problem with regard to this setting. This value
    // must be set to a value in order to load the root object, However we don't
    // know the correct value until after the root object is loaded and
    // deserialized. The value of "false" here is arbitrary. It means that the
    // initial load action for obtaining the root object will search the wrong
    // bucket first about half the time.
    current_bucket_.store(false);
}

bool StoragePlugin_impl::Load(
    const std::string& key,
    const bool checking,
    std::string& value) const
{
    if (key.empty()) {
        if (!checking) {
            std::cout << "Error:: Tried to load empty key" << std::endl;
        }

        return false;
    }

    bool valid = false;
    const bool bucket = current_bucket_.load();

    if (LoadFromBucket(key, value, bucket)) {
        valid = 0 < value.size();
    }

    if (!valid) {
        // try again in the other bucket
        if (LoadFromBucket(key, value, !bucket)) {
            valid = 0 < value.size();
        } else {
            // just in case...
            if (LoadFromBucket(key, value, bucket)) {
                valid = 0 < value.size();
            }
        }
    }

    if (!valid && !checking) {
        otErr << "Specified object is not found." << std::endl
              << "Hash: " << key << std::endl
              << "Size: " << value.size() << std::endl;
    }

    return valid;
}

bool StoragePlugin_impl::Migrate(const std::string& key) const
{
    if (key.empty()) {
        return false;
    }

    std::string value;
    const auto bucket = current_bucket_.load();

    // try to load the key from the inactive bucket
    if (LoadFromBucket(key, value, !bucket)) {

        // save to the active bucket
        if (Store(key, value, bucket)) {
            return true;
        } else {
            otErr << __FUNCTION__ << ": Save failure." << std::endl;
            OT_FAIL;
        }
    }

    // If the key is not in the inactive bucket, it should be in the active
    // bucket
    const bool exists = LoadFromBucket(key, value, bucket);

    if (!exists) {
        otErr << __FUNCTION__ << ": Missing key (" << key
              << "). Database is corrupt." << std::endl;
        OT_FAIL;
    }

    return true;
}

bool StoragePlugin_impl::Store(const std::string& value, std::string& key) const
{
    const bool bucket = current_bucket_.load();

    if (digest_) {
        digest_(Storage::HASH_TYPE, value, key);

        return Store(key, value, bucket);
    }

    return false;
}
}  // namespace opentxs
