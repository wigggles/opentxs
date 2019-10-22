// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"

std::vector<std::unique_ptr<bb::Header>> headers_btc_{};
std::vector<std::unique_ptr<bb::Header>> headers_bch_{};

TEST_F(Test_HeaderOracle, init_opentxs) {}

TEST_F(Test_HeaderOracle, stage_headers)
{
    for (const auto& hex : bitcoin_) {
        const auto raw = ot::Data::Factory(hex, ot::Data::Mode::Hex);
        auto pHeader =
            api_.Factory().BlockHeader(ot::blockchain::Type::Bitcoin, raw);

        ASSERT_TRUE(pHeader);

        headers_btc_.emplace_back(std::move(pHeader));
    }

    for (const auto& hex : bitcoin_) {
        const auto raw = ot::Data::Factory(hex, ot::Data::Mode::Hex);
        auto pHeader =
            api_.Factory().BlockHeader(ot::blockchain::Type::Bitcoin, raw);

        ASSERT_TRUE(pHeader);

        headers_bch_.emplace_back(std::move(pHeader));
    }
}

TEST_F(Test_HeaderOracle, receive_btc)
{
    EXPECT_TRUE(header_oracle_.AddHeaders(headers_btc_));

    const auto [height, hash] = header_oracle_.BestChain();

    EXPECT_EQ(height, bitcoin_.size());

    const auto header = header_oracle_.LoadHeader(hash);

    ASSERT_TRUE(header);

    const auto expectedWork = std::to_string(bitcoin_.size() + 1);

    EXPECT_EQ(expectedWork, header->Work()->Decimal());
}

TEST_F(Test_HeaderOracle, receive_bch)
{
    auto network = std::unique_ptr<bc::internal::Network>(
        ot::Factory::BlockchainNetworkBitcoin(
            api_,
            dynamic_cast<const ot::api::client::internal::Blockchain&>(
                api_.Blockchain()),
            b::Type::BitcoinCash,
            ""));

    ASSERT_TRUE(network);

    auto& oracle = network->HeaderOracle();

    EXPECT_TRUE(oracle.AddHeaders(headers_bch_));

    const auto [height, hash] = oracle.BestChain();

    EXPECT_EQ(height, bitcoin_.size());

    const auto header = oracle.LoadHeader(hash);

    ASSERT_TRUE(header);

    const auto expectedWork = std::to_string(bitcoin_.size() + 1);

    EXPECT_EQ(expectedWork, header->Work()->Decimal());
}
