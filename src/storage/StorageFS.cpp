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

#include <opentxs/storage/StorageFS.hpp>

#include <ios>
#include <iostream>
#include <fstream>
#include <vector>

namespace opentxs
{

StorageFS::StorageFS(const std::string& param, const Digest&hash)
    : ot_super(hash)
{
    Init(param);
}

void StorageFS::Init(const std::string& param)
{
    folder_ = param;
}

std::string StorageFS::LoadRoot()
{
    if (folder_ != "") {
        std::string filename = folder_ + "/root";
        std::ifstream file(
            filename,
            std::ios::in | std::ios::binary | std::ios::ate);

        if (file.is_open()) {
            std::ifstream::pos_type size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<char> bytes(size);
            file.read(&bytes[0], size);

            return std::string(&bytes[0], size);
        }
    }

    return "";
}

bool StorageFS::Load(const std::string& key, std::string& value)
{
    if (folder_ != "") {
        std::string filename = folder_ + "/" + key;
        std::ifstream file(
            filename,
            std::ios::in | std::ios::binary | std::ios::ate);

        if (file.is_open()) {
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
            std::ios::out | std::ios::trunc);

        if (file.is_open()) {
            file.write(hash.c_str(), hash.size());
            file.close();

            return true;
        }
    }

    return false;
}

bool StorageFS::Store(const std::string& key, const std::string& value)
{
    if (folder_ != "") {
        std::string filename = folder_ + "/" + key;
        std::ofstream file(
            filename,
            std::ios::out | std::ios::trunc | std::ios::binary);
        if (file.is_open()) {
            file.write(value.c_str(), value.size());
            file.close();

            return true;
        }
    }

    return false;
}

void StorageFS::Cleanup()
{
}

StorageFS::~StorageFS()
{
    Cleanup();
}


} // namespace opentxs
