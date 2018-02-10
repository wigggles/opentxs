/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include <gtest/gtest.h>
#include <string>

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "opentxs/core/Data.hpp"

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
