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
#if defined(__APPLE__)
#include <fcntl.h>
#else
#include <unistd.h>
#endif
}

#define PATH_SEPERATOR "/"

#define OT_METHOD "opentxs::StorageFS::"

namespace opentxs
{

StorageFS::StorageFS(
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const std::string& folder,
    std::atomic<bool>& bucket)
    : ot_super(config, hash, random, bucket)
    , folder_(folder)
    , path_seperator_(PATH_SEPERATOR)
    , ready_(false)
{
    Init_StorageFS();
}

void StorageFS::Cleanup() { Cleanup_StorageFS(); }

void StorageFS::Cleanup_StorageFS()
{
    // future cleanup actions go here
}

void StorageFS::Init_StorageFS()
{
    // future init actions go here
}

bool StorageFS::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool bucket) const
{
    value.clear();
    std::string directory{};
    const auto filename = calculate_path(key, bucket, directory);

    if (false == boost::filesystem::exists(filename)) {

        return false;
    }

    if (ready_.load() && false == folder_.empty()) {
        value = read_file(filename);
    }

    return false == value.empty();
}

std::string StorageFS::LoadRoot() const
{
    if (ready_.load() && false == folder_.empty()) {

        return read_file(root_filename());
    }

    return "";
}

std::string StorageFS::prepare_read(const std::string& input) const
{
    return input;
}

std::string StorageFS::prepare_write(const std::string& input) const
{
    return input;
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

        return prepare_read(std::string(&bytes[0], size));
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

    if (ready_.load() && false == folder_.empty()) {
        std::string dir{};
        promise->set_value(write_file(calculate_path(key, bucket, dir), value));
    } else {
        promise->set_value(false);
    }
}

bool StorageFS::StoreRoot(const std::string& hash) const
{
    if (ready_.load() && false == folder_.empty()) {

        return write_file(root_filename(), hash);
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
        const auto data = prepare_write(contents);

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

StorageFS::~StorageFS() { Cleanup_StorageFS(); }

}  // namespace opentxs
#endif
