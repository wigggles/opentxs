// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"

#include "Bip158.hpp"

namespace
{
struct Test_BitcoinBlock : public ::testing::Test {
    const ot::api::client::Manager& api_;

    auto ExtractElements(
        const Bip158Vector& vector,
        const ot::blockchain::block::Block& block) const noexcept
        -> std::vector<ot::OTData>
    {
        auto output = std::vector<ot::OTData>{};

        for (const auto& bytes : block.ExtractElements(
                 ot::blockchain::filter::Type::Basic_BIP158)) {
            output.emplace_back(api_.Factory().Data(ot::reader(bytes)));
        }

        const auto& expectedElements = indexed_elements_.at(vector.height_);

        EXPECT_EQ(expectedElements.size(), output.size());

        for (auto& bytes : vector.PreviousOutputs(api_)) {
            output.emplace_back(std::move(bytes));
        }

        return output;
    }

    Test_BitcoinBlock()
        : api_(ot::Context().StartClient({}, 0))
    {
    }
};

TEST_F(Test_BitcoinBlock, instantiate)
{
    for (const auto& vector : bip_158_vectors_) {
        if (926485 == vector.height_) {
            // TODO verify BIP-158 test vectors contain correct hex for block
            // 926485
            continue;
        }

        if (1263442 == vector.height_) {
            // TODO support parsing segwit blocks
            continue;
        }

        const auto raw = vector.Block(api_);
        const auto pBlock = api_.Factory().BitcoinBlock(
            ot::blockchain::Type::Bitcoin_testnet3, raw->Bytes());

        ASSERT_TRUE(pBlock);

        const auto& block = *pBlock;
        const auto pGCS = ot::Factory::GCS(
            api_,
            19,
            784931,
            ot::blockchain::internal::BlockHashToFilterKey(block.ID().Bytes()),
            ExtractElements(vector, block));

        ASSERT_TRUE(pGCS);

        const auto& gcs = *pGCS;
        const auto filter = gcs.Encode();

        EXPECT_EQ(filter.get(), vector.Filter(api_).get());

        namespace bc = ot::blockchain::internal;
        const auto hash = bc::FilterToHash(api_, filter->Bytes());
        const auto header = bc::FilterToHeader(
            api_, filter->Bytes(), vector.PreviousFilterHeader(api_)->Bytes());

        EXPECT_EQ(vector.FilterHeader(api_).get(), header);
    }
}
}  // namespace
