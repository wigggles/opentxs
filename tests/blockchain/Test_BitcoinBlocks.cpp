// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <iosfwd>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Helpers.hpp"
#include "bip158/Bip158.hpp"
#include "bip158/bch_filter_1307544.hpp"
#include "bip158/bch_filter_1307723.hpp"
#include "blockchain/bitcoin/CompactSize.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/core/Data.hpp"

namespace
{
struct Test_BitcoinBlock : public ::testing::Test {
    const ot::api::client::Manager& api_;

    auto CompareElements(
        const std::vector<ot::OTData>& input,
        std::vector<std::string> expected) const -> bool
    {
        auto inputHex = std::vector<std::string>{};
        auto difference = std::vector<std::string>{};
        std::transform(
            std::begin(input), std::end(input), std::back_inserter(inputHex), [
            ](const auto& in) -> auto { return in->asHex(); });

        EXPECT_EQ(expected.size(), inputHex.size());

        if (expected.size() != inputHex.size()) { return false; }

        std::sort(std::begin(inputHex), std::end(inputHex));
        std::sort(std::begin(expected), std::end(expected));
        auto failureCount{0};

        for (auto i{0u}; i < expected.size(); ++i) {
            EXPECT_EQ(expected.at(i), inputHex.at(i));

            if (expected.at(i) != inputHex.at(i)) { ++failureCount; }
        }

        return 0 == failureCount;
    }

    auto ExtractElements(
        const Bip158Vector& vector,
        const ot::blockchain::block::Block& block,
        const std::size_t encodedElements) const noexcept
        -> std::vector<ot::OTData>
    {
        auto output = std::vector<ot::OTData>{};

        for (const auto& bytes : block.ExtractElements(
                 ot::blockchain::filter::Type::Basic_BIP158)) {
            output.emplace_back(api_.Factory().Data(ot::reader(bytes)));
        }

        const auto& expectedElements = indexed_elements_.at(vector.height_);
        auto previousOutputs = vector.PreviousOutputs(api_);

        EXPECT_TRUE(CompareElements(output, expectedElements));

        for (auto& bytes : previousOutputs) {
            if ((nullptr != bytes->data()) && (0 != bytes->size())) {
                output.emplace_back(std::move(bytes));
            }
        }

        {
            auto copy{output};
            std::sort(copy.begin(), copy.end());
            copy.erase(std::unique(copy.begin(), copy.end()), copy.end());

            EXPECT_EQ(encodedElements, copy.size());
        }

        return output;
    }

    Test_BitcoinBlock()
        : api_(ot::Context().StartClient({}, 0))
    {
    }
};

TEST_F(Test_BitcoinBlock, init) {}

TEST_F(Test_BitcoinBlock, btc_genesis_mainnet)
{
    const auto bytes =
        api_.Factory().Data(btc_genesis_block_mainnet_, ot::StringStyle::Hex);
    const auto block = api_.Factory().BitcoinBlock(
        ot::blockchain::Type::Bitcoin, bytes->Bytes());

    ASSERT_TRUE(block);

    const auto gcs = ot::factory::GCS(
        api_, ot::blockchain::filter::Type::Extended_opentxs, *block);

    ASSERT_TRUE(gcs);

    const auto filter = gcs->Encode();
    const auto header =
        ot::blockchain::internal::FilterToHeader(api_, filter->Bytes());

    EXPECT_EQ(filter->asHex(), btc_genesis_block_filter_mainnet_);
    EXPECT_EQ(header->asHex(), btc_genesis_block_filter_header_mainnet_);
}

TEST_F(Test_BitcoinBlock, btc_genesis_testnet)
{
    const auto bytes =
        api_.Factory().Data(btc_genesis_block_testnet_, ot::StringStyle::Hex);
    const auto block = api_.Factory().BitcoinBlock(
        ot::blockchain::Type::Bitcoin_testnet3, bytes->Bytes());

    ASSERT_TRUE(block);

    const auto gcs = ot::factory::GCS(
        api_, ot::blockchain::filter::Type::Extended_opentxs, *block);

    ASSERT_TRUE(gcs);

    const auto filter = gcs->Encode();
    const auto header =
        ot::blockchain::internal::FilterToHeader(api_, filter->Bytes());

    EXPECT_EQ(filter->asHex(), btc_genesis_block_filter_testnet_);
    EXPECT_EQ(header->asHex(), btc_genesis_block_filter_header_testnet_);
}

TEST_F(Test_BitcoinBlock, ltc_genesis_mainnet)
{
    const auto bytes =
        api_.Factory().Data(ltc_genesis_block_mainnet_, ot::StringStyle::Hex);
    const auto block = api_.Factory().BitcoinBlock(
        ot::blockchain::Type::Litecoin, bytes->Bytes());

    ASSERT_TRUE(block);

    const auto gcs = ot::factory::GCS(
        api_, ot::blockchain::filter::Type::Extended_opentxs, *block);

    ASSERT_TRUE(gcs);

    const auto filter = gcs->Encode();
    const auto header =
        ot::blockchain::internal::FilterToHeader(api_, filter->Bytes());

    EXPECT_EQ(filter->asHex(), ltc_genesis_block_filter_mainnet_);
    EXPECT_EQ(header->asHex(), ltc_genesis_block_filter_header_mainnet_);
}

TEST_F(Test_BitcoinBlock, ltc_genesis_testnet)
{
    const auto bytes =
        api_.Factory().Data(ltc_genesis_block_testnet_, ot::StringStyle::Hex);
    const auto block = api_.Factory().BitcoinBlock(
        ot::blockchain::Type::Litecoin_testnet4, bytes->Bytes());

    ASSERT_TRUE(block);

    const auto gcs = ot::factory::GCS(
        api_, ot::blockchain::filter::Type::Extended_opentxs, *block);

    ASSERT_TRUE(gcs);

    const auto filter = gcs->Encode();
    const auto header =
        ot::blockchain::internal::FilterToHeader(api_, filter->Bytes());

    EXPECT_EQ(filter->asHex(), ltc_genesis_block_filter_testnet_);
    EXPECT_EQ(header->asHex(), ltc_genesis_block_filter_header_testnet_);
}

TEST_F(Test_BitcoinBlock, bip158)
{
    for (const auto& vector : bip_158_vectors_) {
        const auto raw = vector.Block(api_);
        const auto pBlock = api_.Factory().BitcoinBlock(
            ot::blockchain::Type::Bitcoin_testnet3, raw->Bytes());

        ASSERT_TRUE(pBlock);

        const auto& block = *pBlock;

        EXPECT_EQ(block.ID(), vector.BlockHash(api_));

        const auto encodedFilter = vector.Filter(api_);
        auto encodedElements = std::size_t{};

        {
            namespace bb = ot::blockchain::bitcoin;
            auto expectedSize = std::size_t{1};
            auto it = static_cast<bb::ByteIterator>(encodedFilter->data());

            ASSERT_TRUE(bb::DecodeCompactSizeFromPayload(
                it, expectedSize, encodedFilter->size(), encodedElements));
        }

        static const auto params = ot::blockchain::internal::GetFilterParams(
            ot::blockchain::filter::Type::Basic_BIP158);
        const auto pGCS = ot::factory::GCS(
            api_,
            params.first,
            params.second,
            ot::blockchain::internal::BlockHashToFilterKey(block.ID().Bytes()),
            ExtractElements(vector, block, encodedElements));

        ASSERT_TRUE(pGCS);

        const auto& gcs = *pGCS;
        const auto filter = gcs.Encode();

        EXPECT_EQ(filter.get(), encodedFilter.get());

        const auto header =
            gcs.Header(vector.PreviousFilterHeader(api_)->Bytes());

        EXPECT_EQ(vector.FilterHeader(api_).get(), header);
    }
}

TEST_F(Test_BitcoinBlock, gcs_headers)
{
    for (const auto& vector : bip_158_vectors_) {
        const auto blockHash = vector.Block(api_);
        const auto encodedFilter = vector.Filter(api_);
        const auto previousHeader = vector.PreviousFilterHeader(api_);

        const auto pGCS = ot::factory::GCS(
            api_,
            ot::blockchain::filter::Type::Basic_BIP158,
            ot::blockchain::internal::BlockHashToFilterKey(blockHash->Bytes()),
            encodedFilter->Bytes());

        ASSERT_TRUE(pGCS);

        const auto& gcs = *pGCS;
        const auto header = gcs.Header(previousHeader->Bytes());

        EXPECT_EQ(header.get(), vector.FilterHeader(api_).get());
    }
}

TEST_F(Test_BitcoinBlock, serialization)
{
    for (const auto& vector : bip_158_vectors_) {
        const auto raw = vector.Block(api_);
        const auto pBlock = api_.Factory().BitcoinBlock(
            ot::blockchain::Type::Bitcoin_testnet3, raw->Bytes());

        ASSERT_TRUE(pBlock);

        const auto& block = *pBlock;
        auto serialized = api_.Factory().Data();

        EXPECT_TRUE(block.Serialize(serialized->WriteInto()));
        EXPECT_EQ(raw.get(), serialized);
    }
}

TEST_F(Test_BitcoinBlock, bch_filter_1307544)
{
    const auto& filter = bch_filter_1307544_;
    const auto blockHash = api_.Factory().Data(
        "a9df8e8b72336137aaf70ac0d390c2a57b2afc826201e9f78b00000000000000",
        ot::StringStyle::Hex);
    const auto encodedFilter = ot::ReadView{
        reinterpret_cast<const char*>(filter.data()), filter.size()};
    const auto previousHeader = api_.Factory().Data(
        "258c5095df5d3d57d4a427add793df679615366ce8ac6e1803a6ea02fca44fc6",
        ot::StringStyle::Hex);
    const auto expectedHeader = api_.Factory().Data(
        "1aa1093ac9289923d390f3bdb2218095dc2d2559f14b4a68b20fcf1656b612b4",
        ot::StringStyle::Hex);

    const auto pGCS = ot::factory::GCS(
        api_,
        ot::blockchain::filter::Type::Basic_BCHVariant,
        ot::blockchain::internal::BlockHashToFilterKey(blockHash->Bytes()),
        encodedFilter);

    ASSERT_TRUE(pGCS);

    const auto& gcs = *pGCS;
    const auto header = gcs.Header(previousHeader->Bytes());

    EXPECT_EQ(header.get(), expectedHeader.get());
}

TEST_F(Test_BitcoinBlock, bch_filter_1307723)
{
    const auto& filter = bch_filter_1307723_;
    const auto blockHash = api_.Factory().Data(
        "c28ca17ec9727809b449447eac0ba416a0b347f3836843f31303000000000000",
        ot::StringStyle::Hex);
    const auto encodedFilter = ot::ReadView{
        reinterpret_cast<const char*>(filter.data()), filter.size()};
    const auto previousHeader = api_.Factory().Data(
        "4417c11a1bfecdbd6948b225dfb92a86021bc2220e1b7d9749af04637b0c9e1f",
        ot::StringStyle::Hex);
    const auto expectedHeader = api_.Factory().Data(
        "747d817e9a7b2130e000b197a08219fa2667c8dc8313591d00492bb9213293ae",
        ot::StringStyle::Hex);

    const auto pGCS = ot::factory::GCS(
        api_,
        ot::blockchain::filter::Type::Basic_BCHVariant,
        ot::blockchain::internal::BlockHashToFilterKey(blockHash->Bytes()),
        encodedFilter);

    ASSERT_TRUE(pGCS);

    const auto& gcs = *pGCS;
    const auto header = gcs.Header(previousHeader->Bytes());

    EXPECT_EQ(header.get(), expectedHeader.get());
}
}  // namespace
