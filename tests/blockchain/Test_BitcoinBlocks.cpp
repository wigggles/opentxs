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

        const auto pGCS = ot::Factory::GCS(
            api_,
            19,
            784931,
            ot::blockchain::internal::BlockHashToFilterKey(block.ID().Bytes()),
            ExtractElements(vector, block, encodedElements));

        ASSERT_TRUE(pGCS);

        const auto& gcs = *pGCS;
        const auto filter = gcs.Encode();

        EXPECT_EQ(filter.get(), encodedFilter.get());

        namespace bc = ot::blockchain::internal;
        const auto hash = bc::FilterToHash(api_, filter->Bytes());
        const auto header = bc::FilterToHeader(
            api_, filter->Bytes(), vector.PreviousFilterHeader(api_)->Bytes());

        EXPECT_EQ(vector.FilterHeader(api_).get(), header);
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
}  // namespace
