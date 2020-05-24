// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"

TEST_F(Test_HeaderOracle, receive_headers_out_of_order)
{
    EXPECT_TRUE(create_blocks(create_5_));
    EXPECT_TRUE(apply_blocks(sequence_5_));
    EXPECT_TRUE(verify_post_state(post_state_5_));
    EXPECT_TRUE(verify_siblings(siblings_5_));

    {
        const auto expected = std::vector<std::string>{
            BLOCK_3,
            BLOCK_12,
            BLOCK_4,
            BLOCK_5,
            BLOCK_6,
            BLOCK_13,
            BLOCK_14,
        };

        EXPECT_TRUE(verify_hash_sequence(3, 0, expected));
        EXPECT_TRUE(verify_hash_sequence(3, 8, expected));
        EXPECT_TRUE(verify_hash_sequence(3, 7, expected));
        EXPECT_FALSE(verify_hash_sequence(3, 6, expected));
    }

    {
        const auto expected = std::vector<std::string>{
            BLOCK_12,
            BLOCK_4,
            BLOCK_5,
        };

        EXPECT_TRUE(verify_hash_sequence(4, 3, expected));
    }

    {
        const auto expected = std::vector<std::string>{
            BLOCK_12,
            BLOCK_5,
            BLOCK_4,
        };

        EXPECT_FALSE(verify_hash_sequence(4, 3, expected));
    }
}
