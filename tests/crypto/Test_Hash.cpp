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
    using HMACVector =
        std::tuple<std::string, std::string, std::string, std::string>;
    using MurmurVector = std::tuple<std::string, std::uint32_t, std::uint32_t>;
    using PbkdfVector = std::
        tuple<std::string, std::string, std::size_t, std::size_t, std::string>;

    static const std::vector<HMACVector> hmac_sha2_;
    static const std::vector<MurmurVector> murmur_;
    static const std::vector<PbkdfVector> pbkdf_;

    const ot::api::Crypto& crypto_;

    Test_Hash()
        : crypto_(ot::Context().Crypto())
    {
    }
};

// https://tools.ietf.org/html/rfc4231
const std::vector<Test_Hash::HMACVector> Test_Hash::hmac_sha2_{
    {"0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b",
     "0x4869205468657265",
     "0xb0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7",
     "0x87aa7cdea5ef619d4ff0b4241a1d6cb02379f4e2ce4ec2787ad0b30545e17cdedaa833b"
     "7d6b8a702038b274eaea3f4e4be9d914eeb61f1702e696c203a126854"},
    {"0x4a656665",
     "0x7768617420646f2079612077616e7420666f72206e6f7468696e673f",
     "0x5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843",
     "0x164b7a7bfcf819e2e395fbe73b56e0a387bd64222e831fd610270cd7ea2505549758bf7"
     "5c05a994a6d034f65f8f0e6fdcaeab1a34d4a6b4b636e070a38bce737"},
    {"0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
     "0xddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd"
     "ddddddddddddddddddddddddddddd",
     "0x773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe",
     "0xfa73b0089d56a284efb0f0756c890be9b1b5dbdd8ee81a3655f83e33b2279d39bf3e848"
     "279a722c806b485a47e67c807b946a337bee8942674278859e13292fb"},
    {"0x0102030405060708090a0b0c0d0e0f10111213141516171819",
     "0xcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdc"
     "dcdcdcdcdcdcdcdcdcdcdcdcdcdcd",
     "0x82558a389a443c0ea4cc819899f2083a85f0faa3e578f8077a2e3ff46729665b",
     "0xb0ba465637458c6990e5a8c5f61d4af7e576d97ff94b872de76f8050361ee3dba91ca5c"
     "11aa25eb4d679275cc5788063a5f19741120c4f2de2adebeb10a298dd"},
    {"0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
     "0x54657374205573696e67204c6172676572205468616e20426c6f636b2d53697a65204b6"
     "579202d2048617368204b6579204669727374",
     "0x60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54",
     "0x80b24263c7c1a3ebb71493c1dd7be8b49b46d1f41b4aeec1121b013783f8f3526b56d03"
     "7e05f2598bd0fd2215d6a1e5295e64f73f63f0aec8b915a985d786598"},
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

// https://tools.ietf.org/html/rfc6070
const std::vector<Test_Hash::PbkdfVector> Test_Hash::pbkdf_{
    {"password", "salt", 1, 20, "0x0c60c80f961f0e71f3a9b524af6012062fe037a6"},
    {"password", "salt", 2, 20, "0xea6c014dc72d6f8ccd1ed92ace1d41f0d8de8957"},
    {"password",
     "salt",
     4096,
     20,
     "0x4b007901b765489abead49d926f721d065a429c1"},
    {"password",
     "salt",
     16777216,
     20,
     "0xeefe3d61cd4da4e4e9945b3d6ba2158c2634e984"},
    {"passwordPASSWORDpassword",
     "saltSALTsaltSALTsaltSALTsaltSALTsalt",
     4096,
     25,
     "0x3d2eec4fe41c849b80c8d83662c0e44a8b291a964cf2f07038"},
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

TEST_F(Test_Hash, PKCS5_PBKDF2_HMAC)
{
    for (const auto& [P, S, c, dkLen, DK] : pbkdf_) {
        const auto salt = ot::Data::Factory(S, ot::Data::Mode::Raw);
        const auto expected = ot::Data::Factory(DK, ot::Data::Mode::Hex);
        auto output = ot::Data::Factory();

        EXPECT_TRUE(crypto_.Hash().PKCS5_PBKDF2_HMAC(
            P, salt, c, ot::proto::HASHTYPE_SHA1, dkLen, output));
        EXPECT_EQ(output, expected);
    }
}

TEST_F(Test_Hash, HMAC_SHA2)
{
    for (const auto& [key, d, sha256, sha512] : hmac_sha2_) {
        const auto data = ot::Data::Factory(d, ot::Data::Mode::Hex);
        const auto dataPassword = ot::Data::Factory(key, ot::Data::Mode::Hex);
        const auto expected256 = ot::Data::Factory(sha256, ot::Data::Mode::Hex);
        const auto expected512 = ot::Data::Factory(sha512, ot::Data::Mode::Hex);
        auto password = ot::OTPassword{};
        auto output256 = ot::OTPassword{};
        auto output512 = ot::OTPassword{};
        password.setMemory(dataPassword->data(), dataPassword->size());

        ASSERT_TRUE(password.isMemory());

        EXPECT_TRUE(crypto_.Hash().HMAC(
            ot::proto::HASHTYPE_SHA256, password, data, output256));
        EXPECT_TRUE(crypto_.Hash().HMAC(
            ot::proto::HASHTYPE_SHA512, password, data, output512));

        ASSERT_TRUE(output256.isMemory());
        ASSERT_TRUE(output512.isMemory());

        const auto data256 =
            ot::Data::Factory(output256.getMemory(), output256.getMemorySize());
        const auto data512 =
            ot::Data::Factory(output512.getMemory(), output512.getMemorySize());

        EXPECT_EQ(data256, expected256);
        EXPECT_EQ(data512, expected512);
    }
}
}  // namespace
