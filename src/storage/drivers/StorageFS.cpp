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

#include "opentxs/storage/drivers/StorageFS.hpp"

#include "opentxs/storage/StorageConfig.hpp"

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <thread>
#include <vector>

extern "C" {
#include <unistd.h>
}

#define OT_METHOD "opentxs::StorageFS::"

namespace opentxs
{

StorageFS::StorageFS(
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    std::atomic<bool>& bucket)
    : ot_super(config, hash, random, bucket)
    , folder_(config.path_)
{
    Init_StorageFS();
}

void StorageFS::Cleanup_StorageFS()
{
    // future cleanup actions go here
}

void StorageFS::Cleanup() { Cleanup_StorageFS(); }

bool StorageFS::EmptyBucket(const bool bucket) const
{
    assert(random_);

    std::string oldDirectory = folder_ + "/" + GetBucketName(bucket);
    std::string random = random_();
    std::string newName = folder_ + "/" + random;

    if (0 != std::rename(oldDirectory.c_str(), newName.c_str())) {
        return false;
    }

    std::thread backgroundDelete(&StorageFS::Purge, this, newName);
    backgroundDelete.detach();

    return boost::filesystem::create_directory(oldDirectory);
}

std::string StorageFS::GetBucketName(const bool bucket) const
{
    return bucket ? config_.fs_secondary_bucket_ : config_.fs_primary_bucket_;
}

void StorageFS::Init_StorageFS()
{
    boost::filesystem::create_directory(
        folder_ + "/" + config_.fs_primary_bucket_);
    boost::filesystem::create_directory(
        folder_ + "/" + config_.fs_secondary_bucket_);
}

bool StorageFS::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool bucket) const
{
    std::string folder = folder_ + "/" + GetBucketName(bucket);
    std::string filename = folder + "/" + key;

    if (false == boost::filesystem::exists(filename)) {
        return false;
    }

    if (false == folder_.empty()) {
        value = read_file(filename);

        return (false == value.empty());
    }

    return false;
}

std::string StorageFS::LoadRoot() const
{
    if (false == folder_.empty()) {
        std::string filename = folder_ + "/" + config_.fs_root_file_;

        return read_file(filename);
    }

    return "";
}

void StorageFS::Purge(const std::string& path) const
{
    if (path.empty()) {
        return;
    }

    boost::filesystem::remove_all(path);
}

std::string StorageFS::read_file(const std::string& filename) const
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

        return std::string(&bytes[0], size);
    }

    return {};
}

void StorageFS::store(
    const std::string& key,
    const std::string& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    OT_ASSERT(nullptr != promise);

    std::string folder = folder_ + "/" + GetBucketName(bucket);
    std::string filename = folder + "/" + key;

    if (false == folder_.empty()) {
        promise->set_value(write_file(filename, value));
    } else {
        promise->set_value(false);
    }
}

bool StorageFS::StoreRoot(const std::string& hash) const
{
    if (false == folder_.empty()) {
        std::string filename = folder_ + "/" + config_.fs_root_file_;

        return write_file(filename, hash);
    }

    return false;
}

bool StorageFS::write_file(
    const std::string& filename,
    const std::string& contents) const
{
    if (false == filename.empty()) {
        boost::filesystem::path filePath(filename);
        boost::iostreams::stream<boost::iostreams::file_descriptor_sink> file(
            filePath);

        if (file.good()) {
            file.write(contents.c_str(), contents.size());
            const auto synced = ::fdatasync(file->handle());

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

StorageFS::~StorageFS() { Cleanup_StorageFS(); }

}  // namespace opentxs
#endif
