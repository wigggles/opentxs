// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#if OPENTXS_BLOCK_STORAGE_ENABLED

#include <boost/iostreams/device/mapped_file.hpp>
#include <iosfwd>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs

namespace opentxs::api::client::blockchain::database::implementation
{
class Blocks
{
public:
    using Hash = opentxs::blockchain::block::Hash;
    using pHash = opentxs::blockchain::block::pHash;

    auto Exists(const Hash& block) const noexcept -> bool;
    auto Load(const Hash& block) const noexcept -> BlockReader;
    auto Store(const Hash& block, const std::size_t bytes) const noexcept
        -> BlockWriter;

    Blocks(
        opentxs::storage::lmdb::LMDB& lmdb,
        const std::string& path) noexcept(false);

private:
    using FileCounter = std::size_t;
    using MemoryPosition = std::size_t;
    using BlockSize = std::size_t;

    struct IndexData {
        MemoryPosition position_;
        BlockSize size_;
    };

    static const std::size_t address_key_;

    opentxs::storage::lmdb::LMDB& lmdb_;
    const std::string path_prefix_;
    mutable MemoryPosition next_position_;
    mutable std::vector<boost::iostreams::mapped_file> files_;
    mutable std::mutex lock_;
    mutable std::map<pHash, std::shared_mutex> block_locks_;

    static auto calculate_file_name(
        const std::string& prefix,
        const FileCounter index) noexcept -> std::string;
    static auto create_or_load(
        const std::string& prefix,
        const FileCounter file,
        std::vector<boost::iostreams::mapped_file>& output) noexcept -> void;
    static auto init_files(
        const std::string& prefix,
        const MemoryPosition position) noexcept
        -> std::vector<boost::iostreams::mapped_file>;
    static auto load_position(opentxs::storage::lmdb::LMDB& db) noexcept
        -> MemoryPosition;

    auto check_file(const Lock& lock, const FileCounter position) const noexcept
        -> void;
};
}  // namespace opentxs::api::client::blockchain::database::implementation
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
