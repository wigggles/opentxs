// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>
#include <gtest/gtest.h>
#include <atomic>
#include <memory>

#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "UIHelpers.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/opentxs.hpp"

namespace
{
Counter counter_{};

class Test_BlockchainSelector : public ::testing::Test
{
public:
    const ot::api::client::Manager& client_;
    const ot::ui::BlockchainSelection& model_;

    Test_BlockchainSelector()
        : client_(ot::Context().StartClient(OTTestEnvironment::test_args_, 0))
        , model_(client_.UI().BlockchainSelection())
    {
    }
};

TEST_F(Test_BlockchainSelector, initialize_opentxs)
{
    counter_.expected_ = 6;
    model_.SetCallback(make_cb(counter_, "Blockchain selector"));

    ASSERT_TRUE(wait_for_counter(counter_));
}

TEST_F(Test_BlockchainSelector, initial_state)
{
    auto row = model_.First();

    ASSERT_TRUE(row->Valid());

    {
        EXPECT_EQ(row->Name(), "Bitcoin");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
    }

    ASSERT_FALSE(row->Last());

    row = model_.Next();

    {
        EXPECT_EQ(row->Name(), "Bitcoin Cash");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::BitcoinCash);
    }

    ASSERT_FALSE(row->Last());

    row = model_.Next();

    {
        EXPECT_EQ(row->Name(), "Litecoin");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Litecoin);
    }

    ASSERT_FALSE(row->Last());

    row = model_.Next();

    {
        EXPECT_EQ(row->Name(), "Bitcoin (testnet3)");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_TRUE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin_testnet3);
    }

    ASSERT_FALSE(row->Last());

    row = model_.Next();

    {
        EXPECT_EQ(row->Name(), "Bitcoin Cash (testnet3)");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_TRUE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::BitcoinCash_testnet3);
    }

    ASSERT_FALSE(row->Last());

    row = model_.Next();

    {
        EXPECT_EQ(row->Name(), "Litecoin (testnet4)");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_TRUE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Litecoin_testnet4);
    }

    EXPECT_TRUE(row->Last());
}

TEST_F(Test_BlockchainSelector, disable_disabled)
{
    EXPECT_TRUE(model_.Disable(ot::blockchain::Type::Bitcoin));

    auto row = model_.First();

    ASSERT_TRUE(row->Valid());

    {
        EXPECT_EQ(row->Name(), "Bitcoin");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
    }

    EXPECT_FALSE(row->Last());
}

TEST_F(Test_BlockchainSelector, enable_disabled)
{
    counter_.expected_ += 2;

    EXPECT_TRUE(model_.Enable(ot::blockchain::Type::Bitcoin));
    ASSERT_TRUE(wait_for_counter(counter_));

    auto row = model_.First();

    ASSERT_TRUE(row->Valid());

    {
        EXPECT_EQ(row->Name(), "Bitcoin");
        EXPECT_TRUE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
    }

    EXPECT_FALSE(row->Last());
}

TEST_F(Test_BlockchainSelector, enable_enabled)
{
    EXPECT_TRUE(model_.Enable(ot::blockchain::Type::Bitcoin));

    auto row = model_.First();

    ASSERT_TRUE(row->Valid());

    {
        EXPECT_EQ(row->Name(), "Bitcoin");
        EXPECT_TRUE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
    }

    EXPECT_FALSE(row->Last());
}

TEST_F(Test_BlockchainSelector, disable_enabled)
{
    counter_.expected_ += 2;

    EXPECT_TRUE(model_.Disable(ot::blockchain::Type::Bitcoin));
    ASSERT_TRUE(wait_for_counter(counter_));

    auto row = model_.First();

    ASSERT_TRUE(row->Valid());

    {
        EXPECT_EQ(row->Name(), "Bitcoin");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
    }

    EXPECT_FALSE(row->Last());
}

TEST_F(Test_BlockchainSelector, toggle_disabled)
{
    counter_.expected_ += 1;

    EXPECT_TRUE(model_.Toggle(ot::blockchain::Type::Bitcoin));
    ASSERT_TRUE(wait_for_counter(counter_));

    auto row = model_.First();

    ASSERT_TRUE(row->Valid());

    {
        EXPECT_EQ(row->Name(), "Bitcoin");
        EXPECT_TRUE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
    }

    EXPECT_FALSE(row->Last());
}

TEST_F(Test_BlockchainSelector, toggle_enabled)
{
    counter_.expected_ += 1;

    EXPECT_TRUE(model_.Toggle(ot::blockchain::Type::Bitcoin));
    ASSERT_TRUE(wait_for_counter(counter_));

    auto row = model_.First();

    ASSERT_TRUE(row->Valid());

    {
        EXPECT_EQ(row->Name(), "Bitcoin");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
    }

    EXPECT_FALSE(row->Last());
}
}  // namespace
