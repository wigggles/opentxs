// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/database/Filters.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iosfwd>
#include <map>
#include <memory>

#include "Factory.hpp"
#include "internal/api/Api.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "util/LMDB.hpp"

// #define OT_METHOD "opentxs::blockchain::database::Filters::"

namespace opentxs::blockchain::database
{
const std::map<
    blockchain::Type,
    std::map<filter::Type, std::pair<std::string, std::string>>>
    Filters::genesis_filters_{
        {blockchain::Type::Bitcoin,
         {
             {filter::Type::Basic_BIP158,
              {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c20"
               "2",
               "017fa880"}},
         }},
        {blockchain::Type::BitcoinCash,
         {
             {filter::Type::Basic_BCHVariant,
              {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c20"
               "2",
               "017fa880"}},
         }},
        {blockchain::Type::Bitcoin_testnet3,
         {
             {filter::Type::Basic_BIP158,
              {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb77945582"
               "1",
               "019dfca8"}},
         }},
        {blockchain::Type::BitcoinCash_testnet3,
         {
             {filter::Type::Basic_BCHVariant,
              {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb77945582"
               "1",
               "019dfca8"}},
         }},
    };

Filters::Filters(
    const api::internal::Core& api,
    const Common& common,
    const opentxs::storage::lmdb::LMDB& lmdb,
    const blockchain::Type chain) noexcept
    : api_(api)
    , common_(common)
    , lmdb_(lmdb)
    , blank_position_(make_blank<block::Position>::value(api))
    , lock_()
{
    import_genesis(chain);
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

auto Filters::import_genesis(const blockchain::Type chain) const noexcept
    -> void
{
    const auto style{blockchain::internal::DefaultFilter(chain)};
    const auto needHeader =
        blank_position_.first == CurrentHeaderTip(style).first;
    const auto needFilter = blank_position_.first == CurrentTip(style).first;

    if (false == (needHeader || needFilter)) { return; }

    const auto pBlock = opentxs::Factory::GenesisBlockHeader(api_, chain);

    OT_ASSERT(pBlock);

    const auto& block = *pBlock;
    const auto& blockHash = block.Hash();
    const auto& genesis = genesis_filters_.at(chain).at(style);
    const auto bytes = api_.Factory().Data(genesis.second, StringStyle::Hex);
    auto gcs = std::unique_ptr<const blockchain::internal::GCS>{Factory::GCS(
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
