// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"

TEST_F(Test_HeaderOracle, basic_sequence)
{
    const auto [heightBefore, hashBefore] = header_oracle_.BestChain();

    EXPECT_EQ(heightBefore, 0);
    EXPECT_EQ(hashBefore, bc::HeaderOracle::GenesisBlockHash(b::Type::Bitcoin));

    EXPECT_TRUE(create_blocks(create_1_));
    EXPECT_TRUE(apply_blocks(sequence_1_));
    EXPECT_TRUE(verify_post_state(post_state_1_));
    EXPECT_TRUE(verify_siblings(siblings_1_));
}
