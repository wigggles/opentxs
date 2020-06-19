// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/client/blockchain/Blockchain.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

namespace blockchain
{
namespace block
{
class Header;
}  // namespace block
}  // namespace blockchain

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
class BlockHeader
{
public:
    auto BlockHeaderExists(
        const opentxs::blockchain::block::Hash& hash) const noexcept -> bool;
    auto LoadBlockHeader(const opentxs::blockchain::block::Hash& hash) const
        noexcept(false) -> proto::BlockchainBlockHeader;
    auto StoreBlockHeader(const opentxs::blockchain::block::Header& header)
        const noexcept -> bool;
    auto StoreBlockHeaders(const UpdatedHeader& headers) const noexcept -> bool;

    BlockHeader(
        const api::client::Manager& api,
        opentxs::storage::lmdb::LMDB& lmdb) noexcept(false);

private:
    const api::client::Manager& api_;
    opentxs::storage::lmdb::LMDB& lmdb_;
};
}  // namespace opentxs::api::client::blockchain::database::implementation
