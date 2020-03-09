// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/Bytes.hpp"

#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "util/LMDB.hpp"

#include <cstdint>

namespace opentxs::api::client::blockchain::database::implementation
{
class BlockFilter
{
public:
    auto HaveFilter(const FilterType type, const ReadView blockHash) const
        noexcept -> bool;
    auto HaveFilterHeader(const FilterType type, const ReadView blockHash) const
        noexcept -> bool;
    auto LoadFilter(const FilterType type, const ReadView blockHash) const
        noexcept -> std::unique_ptr<const opentxs::blockchain::internal::GCS>;
    auto LoadFilterHash(
        const FilterType type,
        const ReadView blockHash,
        const AllocateOutput filterHash) const noexcept -> bool;
    auto LoadFilterHeader(
        const FilterType type,
        const ReadView blockHash,
        const AllocateOutput header) const noexcept -> bool;
    auto StoreFilterHeaders(
        const FilterType type,
        const std::vector<FilterHeader>& headers) const noexcept -> bool;
    auto StoreFilters(const FilterType type, std::vector<FilterData>& filters)
        const noexcept -> bool;

    BlockFilter(
        const api::internal::Core& api,
        opentxs::storage::lmdb::LMDB& lmdb) noexcept(false);

private:
    static const std::uint32_t blockchain_filter_header_version_{1};
    static const std::uint32_t blockchain_filter_headers_version_{1};
    static const std::uint32_t blockchain_filter_version_{1};
    static const std::uint32_t blockchain_filters_version_{1};

    const api::internal::Core& api_;
    opentxs::storage::lmdb::LMDB& lmdb_;

    static auto translate_filter(const FilterType type) noexcept(false)
        -> Table;
    static auto translate_header(const FilterType type) noexcept(false)
        -> Table;
};
}  // namespace opentxs::api::client::blockchain::database::implementation
