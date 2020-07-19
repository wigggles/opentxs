// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/database/Filters.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <utility>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "util/LMDB.hpp"

// #define OT_METHOD "opentxs::blockchain::database::Filters::"

namespace opentxs::blockchain::database
{
Filters::Filters(
    const api::client::Manager& api,
    const Common& common,
    const opentxs::storage::lmdb::LMDB& lmdb,
    const blockchain::Type chain) noexcept
    : api_(api)
    , common_(common)
    , lmdb_(lmdb)
    , blank_position_(make_blank<block::Position>::value(api))
    , lock_()
{
    import_genesis(common_.BlockPolicy(), chain);
}

auto Filters::CurrentHeaderTip(const filter::Type type) const noexcept
    -> block::Position
{
    auto output{blank_position_};
    auto cb = [this, &output](const auto in) {
        output = blockchain::internal::Deserialize(api_, in);
    };
    lmdb_.Load(
        Table::BlockFilterHeaderBest, static_cast<std::size_t>(type), cb);

    return output;
}

auto Filters::CurrentTip(const filter::Type type) const noexcept
    -> block::Position
{
    auto output{blank_position_};
    auto cb = [this, &output](const auto in) {
        output = blockchain::internal::Deserialize(api_, in);
    };
    lmdb_.Load(Table::BlockFilterBest, static_cast<std::size_t>(type), cb);

    return output;
}

auto Filters::import_genesis(
    const api::client::blockchain::BlockStorage mode,
    const blockchain::Type chain) const noexcept -> void
{
    const auto style{
        api::client::blockchain::BlockStorage::All == mode
            ? filter::Type::Extended_opentxs
            : blockchain::internal::DefaultFilter(chain)};
    const auto needHeader =
        blank_position_.first == CurrentHeaderTip(style).first;
    const auto needFilter = blank_position_.first == CurrentTip(style).first;

    if (false == (needHeader || needFilter)) { return; }

    const auto pBlock = factory::GenesisBlockHeader(api_, chain);

    OT_ASSERT(pBlock);

    const auto& block = *pBlock;
    const auto& blockHash = block.Hash();
    const auto& genesis = params::Data::genesis_filters_.at(chain).at(style);
    const auto bytes = api_.Factory().Data(genesis.second, StringStyle::Hex);
    auto gcs = std::unique_ptr<const blockchain::internal::GCS>{factory::GCS(
        api_,
        19,
        784931,
        blockchain::internal::BlockHashToFilterKey(blockHash.Bytes()),
        1,
        bytes->Bytes())};

    OT_ASSERT(gcs);

    const auto filterHash = gcs->Hash();
    auto success{false};

    if (needHeader) {
        auto header = api_.Factory().Data(genesis.first, StringStyle::Hex);
        auto headers = std::vector<client::internal::FilterDatabase::Header>{
            {blockHash, std::move(header), filterHash->Bytes()}};
        success = common_.StoreFilterHeaders(style, headers);

        OT_ASSERT(success);

        success = SetHeaderTip(style, block.Position());

        OT_ASSERT(success);
    }

    if (needFilter) {
        auto filters = std::vector<client::internal::FilterDatabase::Filter>{};
        filters.emplace_back(blockHash.Bytes(), std::move(gcs));

        success = common_.StoreFilters(style, filters);

        OT_ASSERT(success);

        success = SetTip(style, block.Position());

        OT_ASSERT(success);
    }
}

auto Filters::LoadFilterHash(const filter::Type type, const ReadView block)
    const noexcept -> Hash
{
    auto output = api_.Factory().Data();

    if (common_.LoadFilterHash(type, block, output->WriteInto())) {

        return output;
    }

    return api_.Factory().Data();
}

auto Filters::LoadFilterHeader(const filter::Type type, const ReadView block)
    const noexcept -> Hash
{
    auto output = api_.Factory().Data();

    if (common_.LoadFilterHeader(type, block, output->WriteInto())) {

        return output;
    }

    return api_.Factory().Data();
}

auto Filters::SetHeaderTip(
    const filter::Type type,
    const block::Position position) const noexcept -> bool
{
    return lmdb_
        .Store(
            Table::BlockFilterHeaderBest,
            static_cast<std::size_t>(type),
            reader(blockchain::internal::Serialize(position)))
        .first;
}

auto Filters::SetTip(const filter::Type type, const block::Position position)
    const noexcept -> bool
{
    return lmdb_
        .Store(
            Table::BlockFilterBest,
            static_cast<std::size_t>(type),
            reader(blockchain::internal::Serialize(position)))
        .first;
}
}  // namespace opentxs::blockchain::database
