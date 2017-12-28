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
#include "opentxs/stdafx.hpp"

#include "opentxs/storage/drivers/StorageFSArchive.hpp"

#if OT_STORAGE_FS
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/storage/StorageConfig.hpp"

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <cstdio>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <thread>
#include <vector>

#if defined(__APPLE__)
extern "C" {
#include <fcntl.h>
}
#endif

extern "C" {
#include <unistd.h>
}

#define ROOT_FILE_EXTENSION ".hash"

#define OT_METHOD "opentxs::StorageFSArchive::"

namespace opentxs
{

StorageFSArchive::StorageFSArchive(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const std::atomic<bool>& bucket,
    const std::string& folder,
    std::unique_ptr<SymmetricKey>& key)
    : ot_super(storage, config, hash, random, folder, bucket)
    , encryption_key_(key.release())
    , encrypted_(bool(encryption_key_))
{
    Init_StorageFSArchive();
}

std::string StorageFSArchive::calculate_path(
    const std::string& key,
    const bool,
    std::string& directory) const
{
    directory = folder_;
    auto& level1 = folder_;
    std::string level2{};

    if (4 < key.size()) {
        directory += path_seperator_;
        directory += key.substr(0, 4);
        level2 = directory;
    }

    if (8 < key.size()) {
        directory += path_seperator_;
        directory += key.substr(4, 4);
    }

    boost::system::error_code ec{};
    boost::filesystem::create_directories(directory, ec);

    if (8 < key.size()) {
        if (false == sync(level2)) {
            otErr << OT_METHOD << __FUNCTION__ << ": Unable to sync directory"
                  << level2 << std::endl;
        }
    }

    if (false == sync(level1)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to sync directory"
              << level1 << std::endl;
    }

    return {directory + path_seperator_ + key};
}

void StorageFSArchive::Cleanup()
{
    Cleanup_StorageFSArchive();
    ot_super::Cleanup();
}

void StorageFSArchive::Cleanup_StorageFSArchive()
{
    // future cleanup actions go here
}

bool StorageFSArchive::EmptyBucket(const bool) const { return true; }

void StorageFSArchive::Init_StorageFSArchive()
{
    OT_ASSERT(false == folder_.empty());

    boost::system::error_code ec{};

    if (boost::filesystem::create_directory(folder_, ec)) {
        ready_.store(true);
    }
}

std::string StorageFSArchive::prepare_read(const std::string& input) const
{
    if (false == encrypted_) {

        return input;
    }

    const auto ciphertext = proto::TextToProto<proto::Ciphertext>(input);

    OT_ASSERT(encryption_key_);

    std::string output{};
    OTPasswordData reason("");

    if (false == encryption_key_->Decrypt(ciphertext, reason, output)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to decrypt value."
              << std::endl;
    }

    return output;
}

std::string StorageFSArchive::prepare_write(const std::string& plaintext) const
{
    if (false == encrypted_) {

        return plaintext;
    }

    OT_ASSERT(encryption_key_);

    proto::Ciphertext ciphertext{};
    Data iv{};
    OTPasswordData reason("");
    const bool encrypted =
        encryption_key_->Encrypt(plaintext, iv, reason, ciphertext, false);

    if (false == encrypted) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to encrypt value."
              << std::endl;
    }

    return proto::ProtoAsString(ciphertext);
}

std::string StorageFSArchive::root_filename() const
{
    return folder_ + path_seperator_ + config_.fs_root_file_ +
           ROOT_FILE_EXTENSION;
}

StorageFSArchive::~StorageFSArchive() { Cleanup_StorageFSArchive(); }
}  // namespace opentxs
#endif
