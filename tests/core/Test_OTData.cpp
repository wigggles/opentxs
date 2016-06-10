#include <gtest/gtest.h>
#include "opentxs/core/OTData.hpp"

using namespace opentxs;

namespace
{

struct Default_OTData : public ::testing::Test
{
  OTData data_;
};

} // namespace

TEST_F(Default_OTData, default_accessors)
{
  ASSERT_TRUE(data_.GetPointer() == 0);
  ASSERT_TRUE(data_.GetSize() == 0);
}

TEST(OTData, compare_equal_to_self)
{
    OTData one("abcd", 4);
    ASSERT_TRUE(one == one);
}

TEST(OTData, compare_equal_to_other_same)
{
    OTData one("abcd", 4);
    OTData other("abcd", 4);
    ASSERT_TRUE(one == other);
}

TEST(OTData, compare_equal_to_other_different)
{
    OTData one("abcd", 4);
    OTData other("zzzz", 4);
    ASSERT_FALSE(one == other);
}

TEST(OTData, compare_not_equal_to_self)
{
    OTData one("aaaa", 4);
    ASSERT_FALSE(one != one);
}

TEST(OTData, compare_not_equal_to_other_same)
{
    OTData one("abcd", 4);
    OTData other("abcd", 4);
    ASSERT_FALSE(one != other);
}

TEST(OTData, compare_not_equal_to_other_different)
{
    OTData one("abcd", 4);
    OTData other("zzzz", 4);
    ASSERT_TRUE(one != other);
}
