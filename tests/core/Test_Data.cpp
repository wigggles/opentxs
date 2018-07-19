// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{
struct Default_Data : public ::testing::Test {
    OTData data_;

    Default_Data()
        : data_(Data::Factory())
    {
    }
};
}  // namespace

TEST_F(Default_Data, default_accessors)
{
    ASSERT_EQ(data_->GetPointer(), nullptr);
    ASSERT_EQ(data_->GetSize(), 0);
}

TEST(Data, compare_equal_to_self)
{
    auto one = Data::Factory("abcd", 4);
    ASSERT_TRUE(one == one);
}

TEST(Data, compare_equal_to_other_same)
{
    auto one = Data::Factory("abcd", 4);
    auto other = Data::Factory("abcd", 4);
    ASSERT_TRUE(one == other);
}

TEST(Data, compare_equal_to_other_different)
{
    auto one = Data::Factory("abcd", 4);
    auto other = Data::Factory("zzzz", 4);
    ASSERT_FALSE(one == other);
}

TEST(Data, compare_not_equal_to_self)
{
    auto one = Data::Factory("aaaa", 4);
    ASSERT_FALSE(one != one);
}

TEST(Data, compare_not_equal_to_other_same)
{
    auto one = Data::Factory("abcd", 4);
    auto other = Data::Factory("abcd", 4);
    ASSERT_FALSE(one != other);
}

TEST(Data, compare_not_equal_to_other_different)
{
    auto one = Data::Factory("abcd", 4);
    auto other = Data::Factory("zzzz", 4);
    ASSERT_TRUE(one != other);
}

TEST(Data, copy_from_pimpl)
{
    auto one = Data::Factory("abcd", 4);
    auto other = Data::Factory(one);
    std::string value(
        static_cast<const char*>(other->GetPointer()), other->GetSize());
    ASSERT_EQ(value, "abcd");
}

TEST(Data, copy_from_interface)
{
    auto one = Data::Factory("abcd", 4);
    auto other = Data::Factory(one.get());
    std::string value(
        static_cast<const char*>(other->GetPointer()), other->GetSize());
    ASSERT_EQ(value, "abcd");
}
