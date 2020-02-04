// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "internal/api/client/blockchain/Blockchain.hpp"
#include "util/LMDB.hpp"

namespace opentxs::api::client::blockchain::database::implementation
{
class BlockHeader
{
public:
    auto BlockHeaderExists(const opentxs::blockchain::block::Hash& hash) const
        noexcept -> bool;
    auto LoadBlockHeader(const opentxs::blockchain::block::Hash& hash) const
        noexcept(false) -> proto::BlockchainBlockHeader;
    auto StoreBlockHeader(const opentxs::blockchain::block::Header& header)
        const noexcept -> bool;
    auto StoreBlockHeaders(const UpdatedHeader& headers) const noexcept -> bool;

    BlockHeader(
        const api::internal::Core& api,
        opentxs::storage::lmdb::LMDB& lmdb) noexcept(false);

private:
    const api::internal::Core& api_;
    opentxs::storage::lmdb::LMDB& lmdb_;
};
}  // namespace opentxs::api::client::blockchain::database::implementation
