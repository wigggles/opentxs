// Copyright (c) 2010-2020 The Open-Transactions developers
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

TEST_F(Test_Filters, bip158_headers)
{
    namespace bc = ot::blockchain::internal;

    const auto filter_0 =
        api_.Factory().Data("0x019dfca8", ot::StringStyle::Hex);
    const auto filter_1 =
        api_.Factory().Data("0x015d5000", ot::StringStyle::Hex);
    const auto filter_2 =
        api_.Factory().Data("0x0174a170", ot::StringStyle::Hex);
    const auto filter_3 =
        api_.Factory().Data("0x016cf7a0", ot::StringStyle::Hex);
    const auto blank = api_.Factory().Data(
        "0x0000000000000000000000000000000000000000000000000000000000000000",
        ot::StringStyle::Hex);
    const auto expected_0 = api_.Factory().Data(
        "0x50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb779455821",
        ot::StringStyle::Hex);
    const auto expected_1 = api_.Factory().Data(
        "0xe14fc288fdbf3c8d84f31bfc45892e44a0f152e82c0ddd1a5b749da513acbdd7",
        ot::StringStyle::Hex);
    const auto expected_2 = api_.Factory().Data(
        "0xf06c381b7d46b1f8df603de51f25fda128dff8cbe8f204357e5e2bef11fd6a18",
        ot::StringStyle::Hex);
    const auto expected_3 = api_.Factory().Data(
        "0x2a9d721212af044cec24f188631cff7b516fb1576a31d2b67c25b75adfaa638d",
        ot::StringStyle::Hex);
    auto previous = blank->Bytes();
    auto hash = bc::FilterToHash(api_, filter_0->Bytes());
    auto calculated_a = bc::FilterHashToHeader(api_, hash->Bytes(), previous);
    auto calculated_b = bc::FilterToHeader(api_, filter_0->Bytes(), previous);

    EXPECT_EQ(calculated_a.get(), calculated_b.get());
    EXPECT_EQ(calculated_a.get(), expected_0.get());

    previous = expected_0->Bytes();
    hash = bc::FilterToHash(api_, filter_1->Bytes());
    calculated_a = bc::FilterHashToHeader(api_, hash->Bytes(), previous);
    calculated_b = bc::FilterToHeader(api_, filter_1->Bytes(), previous);

    EXPECT_EQ(calculated_a.get(), calculated_b.get());
    EXPECT_EQ(calculated_a.get(), expected_1.get());

    previous = expected_1->Bytes();
    hash = bc::FilterToHash(api_, filter_2->Bytes());
    calculated_a = bc::FilterHashToHeader(api_, hash->Bytes(), previous);
    calculated_b = bc::FilterToHeader(api_, filter_2->Bytes(), previous);

    EXPECT_EQ(calculated_a.get(), calculated_b.get());
    EXPECT_EQ(calculated_a.get(), expected_2.get());

    previous = expected_2->Bytes();
    hash = bc::FilterToHash(api_, filter_3->Bytes());
    calculated_a = bc::FilterHashToHeader(api_, hash->Bytes(), previous);
    calculated_b = bc::FilterToHeader(api_, filter_3->Bytes(), previous);

    EXPECT_EQ(calculated_a.get(), calculated_b.get());
    EXPECT_EQ(calculated_a.get(), expected_3.get());
}

TEST_F(Test_Filters, hash)
{
    namespace bc = ot::blockchain::internal;

    const auto& block_0 =
        ot::blockchain::client::HeaderOracle::GenesisBlockHash(
            ot::blockchain::Type::Bitcoin_testnet3);
    const auto preimage =
        api_.Factory().Data("0x019dfca8", ot::StringStyle::Hex);
    const auto filter_0 = api_.Factory().Data("0x9dfca8", ot::StringStyle::Hex);
    auto pGcs = std::unique_ptr<bc::GCS>{ot::Factory::GCS(
        api_,
        19,
        784931,
        bc::BlockHashToFilterKey(block_0.Bytes()),
        1,
        filter_0)};

    ASSERT_TRUE(pGcs);

    const auto& gcs = *pGcs;
    const auto hash_a = gcs.Hash();
    const auto hash_b = bc::FilterToHash(api_, preimage->Bytes());

    EXPECT_EQ(hash_a.get(), hash_b.get());
}
}  // namespace
