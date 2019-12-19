// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/String.hpp"

#include "internal/blockchain/client/Client.hpp"

#include "util/LMDB.hpp"

namespace opentxs::api::client::blockchain::database::implementation
{
class Database final
{
public:
    using UpdatedHeader = opentxs::blockchain::client::UpdatedHeader;

    std::string AllocateStorageFolder(const std::string& dir) const noexcept;
    bool BlockHeaderExists(const opentxs::blockchain::block::Hash& hash) const
        noexcept;
    proto::BlockchainBlockHeader LoadBlockHeader(
        const opentxs::blockchain::block::Hash& hash) const noexcept(false);
    bool StoreBlockHeader(
        const opentxs::blockchain::block::Header& header) const noexcept;
    bool StoreBlockHeaders(const UpdatedHeader& headers) const noexcept;

    Database(
        const api::internal::Core& api,
        const api::Legacy& legacy,
        const std::string& dataFolder) noexcept(false);

private:
    enum Table {
        BlockHeaders = 0,
    };

    static const opentxs::storage::lmdb::TableNames table_names_;

    const api::internal::Core& api_;
    const OTString blockchain_path_;
    const OTString common_path_;
    opentxs::storage::lmdb::LMDB lmdb_;

    static OTString init_folder(
        const api::Legacy& legacy,
        const String& parent,
        const String& child) noexcept(false);
    static OTString init_storage_path(
        const api::Legacy& legacy,
        const std::string& dataFolder) noexcept(false);

    Database() = delete;
    Database(const Database&) = delete;
    Database(Database&&) = delete;
    Database& operator=(const Database&) = delete;
    Database& operator=(Database&&) = delete;
};
}  // namespace opentxs::api::client::blockchain::database::implementation
