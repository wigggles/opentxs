// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"

#include <algorithm>

std::vector<std::unique_ptr<bb::Header>> headers_{};

TEST_F(Test_HeaderOracle, init_opentxs) {}

TEST_F(Test_HeaderOracle, stage_headers)
{
    for (const auto& hex : bitcoin_) {
        const auto raw = ot::Data::Factory(hex, ot::Data::Mode::Hex);
        auto pHeader =
            api_.Factory().BlockHeader(ot::blockchain::Type::Bitcoin, raw);

        ASSERT_TRUE(pHeader);

        headers_.emplace_back(std::move(pHeader));
    }

    std::random_shuffle(headers_.begin(), headers_.end());
}

TEST_F(Test_HeaderOracle, receive)
{
    EXPECT_TRUE(header_oracle_.AddHeaders(headers_));

    const auto [height, hash] = header_oracle_.BestChain();

    EXPECT_EQ(height, bitcoin_.size());

    const auto header = header_oracle_.LoadHeader(hash);

    ASSERT_TRUE(header);

    const auto expectedWork = std::to_string(bitcoin_.size() + 1);

    EXPECT_EQ(expectedWork, header->Work()->Decimal());
}
