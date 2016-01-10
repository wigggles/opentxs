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
#ifdef OT_STORAGE_FS
#include <opentxs/storage/StorageFS.hpp>

#include <cstdio>
#include <ios>
#include <iostream>
#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>

namespace opentxs
{

StorageFS::StorageFS(
    const std::string& param,
    const Digest&hash,
    const Random& random)
        : ot_super(hash, random)
{
    Init(param);
}

void StorageFS::Init(const std::string& param)
{
    folder_ = param;
}

void StorageFS::Purge(const std::string& path)
{
    if (path.empty()) { return; }

    boost::filesystem::remove_all(path);
}

std::string StorageFS::LoadRoot()
{
    if (folder_ != "") {
        std::string filename = folder_ + "/root";
        std::ifstream file(
            filename,
            std::ios::in | std::ios::ate | std::ios::binary);

        if (file.good()) {
            std::ifstream::pos_type size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<char> bytes(size);
            file.read(&bytes[0], size);

            return std::string(&bytes[0], size);
        }
    }

    return "";
}

bool StorageFS::Load(
    const std::string& key,
    std::string& value,
    const bool altLocation)
{
    std::string folder =  folder_ + "/" + GetBucketName(altLocation);
    std::string filename = folder + "/" + key;

    if (folder_ != "") {
        std::ifstream file(
            filename,
            std::ios::in | std::ios::ate | std::ios::binary);

        if (file.good()) {
            std::ifstream::pos_type size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<char> bytes(size);
            file.read(&bytes[0], size);

            value.assign(&bytes[0], size);

            return true;
        }
    }

    return false;
}

bool StorageFS::StoreRoot(const std::string& hash)
{
    if (folder_ != "") {
        std::string filename = folder_ + "/root";
        std::ofstream file(
            filename,
            std::ios::out | std::ios::trunc | std::ios::binary);

        if (file.good()) {
            file.write(hash.c_str(), hash.size());
            file.close();

            return true;
        }
    }

    return false;
}

bool StorageFS::Store(
    const std::string& key,
    const std::string& value,
    const bool altLocation)
{
    std::string folder =  folder_ + "/" + GetBucketName(altLocation);
    std::string filename = folder + "/" + key;

    if (folder_ != "") {
        std::ofstream file(
            filename,
            std::ios::out | std::ios::trunc | std::ios::binary);
        if (file.good()) {
            file.write(value.c_str(), value.size());
            file.close();

            return true;
        }
    }

    return false;
}

bool StorageFS::EmptyBucket(
    const bool altLocation)
{
    assert(nullptr != random_);

    std::string oldDirectory = folder_ + "/" + GetBucketName(altLocation);
    std::string random = random_();
    std::string newName = folder_ + "/" + random;

    if (0 != std::rename(oldDirectory.c_str(), newName.c_str())) {
        return false;
    }

    std::thread backgroundDelete(&StorageFS::Purge, this, newName);
    backgroundDelete.detach();

    return boost::filesystem::create_directory(oldDirectory);
}

void StorageFS::Cleanup()
{
    ot_super::Cleanup();
}

StorageFS::~StorageFS()
{
    Cleanup();
}

} // namespace opentxs
#endif
