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
#if OT_STORAGE_FS
#include "opentxs/core/stdafx.hpp"

#include "opentxs/storage/drivers/StorageFSArchive.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Proto.hpp"
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
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    std::atomic<bool>& bucket,
    const std::string& folder,
    std::unique_ptr<SymmetricKey>& key)
    : ot_super(config, hash, random, bucket)
    , folder_(folder)
    , path_seperator_("/")
    , encryption_key_(key.release())
    , encrypted_(bool(encryption_key_))
    , ready_(false)
{
    Init_StorageFSArchive();
}

std::string StorageFSArchive::calculate_path(const std::string& key) const
{
    std::string folder(folder_);

    if (4 < key.size()) {
        folder += path_seperator_;
        folder += key.substr(0, 4);
    }

    if (8 < key.size()) {
        folder += path_seperator_;
        folder += key.substr(4, 4);
    }

    boost::filesystem::create_directories(folder);

    return {folder + path_seperator_ + key};
}

void StorageFSArchive::Cleanup() { Cleanup_StorageFSArchive(); }

void StorageFSArchive::Cleanup_StorageFSArchive()
{
    // future cleanup actions go here
}

std::string StorageFSArchive::decrypt(const std::string&& input) const
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

bool StorageFSArchive::EmptyBucket(const bool) const { return true; }

std::string StorageFSArchive::encrypt(const std::string& plaintext) const
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

void StorageFSArchive::Init_StorageFSArchive()
{
    OT_ASSERT(false == folder_.empty());

    try {
        boost::filesystem::create_directory(folder_);
    } catch (boost::filesystem::filesystem_error&) {
        return;
    }

    ready_.store(true);
}

bool StorageFSArchive::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool) const
{
    value.clear();

    if (ready_.load() && (false == folder_.empty())) {
        value = read_file(calculate_path(key));
    }

    return (false == value.empty());
}

std::string StorageFSArchive::LoadRoot() const
{
    if (ready_.load() && (false == folder_.empty())) {
        std::string filename = folder_ + path_seperator_ +
                               config_.fs_root_file_ + ROOT_FILE_EXTENSION;

        return read_file(filename);
    }

    return "";
}

std::string StorageFSArchive::read_file(const std::string& filename) const
{
    if (false == boost::filesystem::exists(filename)) {
        return {};
    }

    std::ifstream file(
        filename, std::ios::in | std::ios::ate | std::ios::binary);

    if (file.good()) {
        std::ifstream::pos_type pos = file.tellg();

        if ((0 >= pos) || (0xFFFFFFFF <= pos)) {
            return {};
        }

        std::uint32_t size(pos);
        file.seekg(0, std::ios::beg);
        std::vector<char> bytes(size);
        file.read(&bytes[0], size);

        return decrypt(std::string(&bytes[0], size));
    }

    return {};
}

void StorageFSArchive::store(
    const std::string& key,
    const std::string& value,
    const bool,
    std::promise<bool>* promise) const
{
    OT_ASSERT(nullptr != promise);

    if (ready_.load() && false == folder_.empty()) {
        promise->set_value(write_file(calculate_path(key), value));
    } else {
        promise->set_value(false);
    }
}

bool StorageFSArchive::StoreRoot(const std::string& hash) const
{
    if (ready_.load() && (false == folder_.empty())) {
        const std::string filename = folder_ + path_seperator_ +
                                     config_.fs_root_file_ +
                                     ROOT_FILE_EXTENSION;

        return write_file(filename, hash);
    }

    return false;
}

bool StorageFSArchive::write_file(
    const std::string& filename,
    const std::string& contents) const
{
    if (false == filename.empty()) {
        boost::filesystem::path filePath(filename);
        boost::iostreams::stream<boost::iostreams::file_descriptor_sink> file(
            filePath);
        const std::string data = encrypt(contents);

        if (file.good()) {
            file.write(data.c_str(), data.size());

#ifdef F_FULLFSYNC
            // This is a Mac OS X system which does not implement
            // fdatasync as such.
            const auto synced = fcntl(file->handle(), F_FULLFSYNC);
#else
            const auto synced = ::fdatasync(file->handle());
#endif
            if (0 != synced) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed to flush file buffer." << std::endl;
            }

            file.close();

            return true;
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Failed to write file."
                  << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to write empty filename." << std::endl;
    }

    return false;
}

StorageFSArchive::~StorageFSArchive() { Cleanup_StorageFSArchive(); }
}  // namespace opentxs
#endif
