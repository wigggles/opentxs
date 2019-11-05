// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"

namespace
{
class Test_Filters : public ::testing::Test
{
public:
    const ot::api::client::internal::Manager& api_;

    Test_Filters()
        : api_(dynamic_cast<const ot::api::client::internal::Manager&>(
              ot::Context().StartClient(OTTestEnvironment::test_args_, 0)))
    {
    }
};

TEST_F(Test_Filters, bloom_filter)
{
    std::string s1("blah");
    std::string s2("foo");
    std::string s3("justus");
    std::string s4("fellowtraveler");

    const auto object1(ot::Data::Factory(s1.data(), s1.length()));
    const auto object2(ot::Data::Factory(s2.data(), s2.length()));
    const auto object3(ot::Data::Factory(s3.data(), s3.length()));
    const auto object4(ot::Data::Factory(s4.data(), s4.length()));

    ot::OTBloomFilter pFilter{ot::Factory::BloomFilter(
        api_, 9873485, ot::blockchain::BloomUpdateFlag::None, 5, 0.001)};

    pFilter->AddElement(object1);
    pFilter->AddElement(object4);

    EXPECT_TRUE(pFilter->Test(object1));
    EXPECT_FALSE(pFilter->Test(object2));
    EXPECT_FALSE(pFilter->Test(object3));
    EXPECT_TRUE(pFilter->Test(object4));
}

TEST_F(Test_Filters, bitstreams)
{
    namespace be = boost::endian;

    auto data = ot::Data::Factory();

    const std::uint64_t v1{19};
    const std::uint64_t v2{3491};
    const std::uint64_t v3{15000};
    const std::uint64_t v4{1073027};
    const std::uint64_t v5{28998};

    ot::blockchain::internal::BitWriter stream(data);

    stream.write(19, v1);
    stream.write(31, v2);
    stream.write(23, v3);
    stream.write(31, v4);
    stream.write(19, v5);
    stream.flush();

    ot::blockchain::internal::BitReader reader(data);

    const std::uint64_t o1 = reader.read(19);
    const std::uint64_t o2 = reader.read(31);
    const std::uint64_t o3 = reader.read(23);
    const std::uint64_t o4 = reader.read(31);
    const std::uint64_t o5 = reader.read(19);

    EXPECT_TRUE(o1 == v1);
    EXPECT_TRUE(o2 == v2);
    EXPECT_TRUE(o3 == v3);
    EXPECT_TRUE(o4 == v4);
    EXPECT_TRUE(o5 == v5);
}

TEST_F(Test_Filters, gcs)
{
    std::string s1("blah");
    std::string s2("foo");
    std::string s3("justus");
    std::string s4("fellowtraveler");
    std::string s5("islajames");
    std::string s6("timewaitsfornoman");

    const auto object1(ot::Data::Factory(s1.data(), s1.length()));
    const auto object2(ot::Data::Factory(s2.data(), s2.length()));
    const auto object3(ot::Data::Factory(s3.data(), s3.length()));
    const auto object4(ot::Data::Factory(s4.data(), s4.length()));
    const auto object5(ot::Data::Factory(s5.data(), s5.length()));
    const auto object6(ot::Data::Factory(s6.data(), s6.length()));

    std::vector<ot::OTData> includedElements{
        object1, object2, object3, object4};
    std::vector<ot::OTData> excludedElements{object5, object6};
    std::array<std::byte, 16> key{};

    for (std::size_t ii = 0; ii < 16; ii++) {
        key[ii] = static_cast<std::byte>(1);
    }

    std::unique_ptr<ot::blockchain::internal::GCS> pGcs{
        ot::Factory::GCS(api_, 19, 784931, key, includedElements)};
    ASSERT_TRUE(pGcs);
    const auto& gcs = *pGcs;
    EXPECT_TRUE(gcs.Test(object1));
    EXPECT_TRUE(gcs.Test(object2));
    EXPECT_TRUE(gcs.Test(object3));
    EXPECT_TRUE(gcs.Test(object4));

    EXPECT_FALSE(gcs.Test(object5));
    EXPECT_FALSE(gcs.Test(object6));

    EXPECT_TRUE(gcs.Test(includedElements));
    EXPECT_FALSE(gcs.Test(excludedElements));
}
}  // namespace
