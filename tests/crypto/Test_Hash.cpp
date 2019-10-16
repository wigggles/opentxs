// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"

namespace
{
class Test_Hash : public ::testing::Test
{
public:
    using MurmurVector = std::tuple<std::string, std::uint32_t, std::uint32_t>;

    static const std::vector<MurmurVector> murmur_;

    const ot::api::Crypto& crypto_;

    Test_Hash()
        : crypto_(ot::Context().Crypto())
    {
    }
};

// https://stackoverflow.com/a/31929528
const std::vector<Test_Hash::MurmurVector> Test_Hash::murmur_{
    {"", 0, 0},
    {"", 1, 1364076727},
    {"", 4294967295, 2180083513},
    {"0xffffffff", 0, 1982413648},
    {"0x21436587", 0, 4116402539},
    {"0x21436587", 1350757870, 593689054},
    {"0x214365", 0, 2118813236},
    {"0x2143", 0, 2700587130},
    {"0x21", 0, 1919294708},
    {"0x00000000", 0, 593689054},
    {"0x000000", 0, 2247144487},
    {"0x0000", 0, 821347078},
    {"0x00", 0, 1364076727},
};

TEST_F(Test_Hash, MurmurHash3)
{
    for (const auto& [input, seed, expected] : murmur_) {
        std::uint32_t calculated{0};
        const auto plaintext = ot::Data::Factory(input, ot::Data::Mode::Hex);
        crypto_.Hash().MurmurHash3_32(seed, plaintext, calculated);

        EXPECT_EQ(calculated, expected);
    }
}

TEST_F(Test_Hash, SipHash)
{
    const auto plaintext = ot::Data::Factory("hello", ot::Data::Mode::Raw);
    const std::string keyString{"0123456789ABCDEF"};
    const auto key = ot::OTPassword(keyString.data(), keyString.size());
    const std::uint64_t expected1{4402678656023170274UL};
    const std::uint64_t expected2{14986662229302055855UL};
    std::uint64_t output{};

    EXPECT_TRUE(crypto_.Hash().SipHash(key, plaintext, output, 2, 4));
    EXPECT_EQ(output, expected1);

    EXPECT_TRUE(crypto_.Hash().SipHash(key, plaintext, output, 4, 8));
    EXPECT_EQ(output, expected2);
}
}  // namespace
