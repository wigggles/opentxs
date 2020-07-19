// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/protobuf/Enums.pb.h"

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
    using SiphashVector =
        std::tuple<int, int, std::string, std::string, std::uint64_t>;
    // input, salt, N, r, p, size, expected hex
    using ScryptVector = std::tuple<
        std::string,
        std::string,
        std::uint64_t,
        std::uint32_t,
        std::uint32_t,
        std::size_t,
        std::string>;

    static const std::vector<HMACVector> hmac_sha2_;
    static const std::vector<MurmurVector> murmur_;
    static const std::vector<PbkdfVector> pbkdf_;
    static const std::vector<ScryptVector> scrypt_rfc7914_;
    static const std::vector<ScryptVector> scrypt_litecoin_;

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

// https://tools.ietf.org/html/rfc7914
const std::vector<Test_Hash::ScryptVector> Test_Hash::scrypt_rfc7914_{
    {"",
     "",
     16,
     1,
     1,
     64,
     "77d6576238657b203b19ca42c18a0497f16b4844e3074ae8dfdffa3fede21442fcd0069de"
     "d0948f8326a753a0fc81f17e8d3e0fb2e0d3628cf35e20c38d18906"},
    {"password",
     "NaCl",
     1024,
     8,
     16,
     64,
     "fdbabe1c9d3472007856e7190d01e9fe7c6ad7cbc8237830e77376634b3731622eaf30d92"
     "e22a3886ff109279d9830dac727afb94a83ee6d8360cbdfa2cc0640"},
    {"pleaseletmein",
     "SodiumChloride",
     16384,
     8,
     1,
     64,
     "7023bdcb3afd7348461c06cd81fd38ebfda8fbba904f8e3ea9b543f6545da1f2d54329556"
     "13f0fcf62d49705242a9af9e61e85dc0d651e40dfcf017b45575887"},
    {"pleaseletmein",
     "SodiumChloride",
     1048576,
     8,
     1,
     64,
     "2101cb9b6a511aaeaddbbe09cf70f881ec568d574a2ffd4dabe5ee9820adaa478e56fd8f4"
     "ba5d09ffa1c6d927c40f4c337304049e8a952fbcbf45c6fa77a41a4"},
};

// https://www.litecoin.info/index.php/Block_hashing_algorithm
const std::vector<Test_Hash::ScryptVector> Test_Hash::scrypt_litecoin_{
    {"01000000ae178934851bfa0e83ccb6a3fc4bfddff3641e104b6c4680c31509074e699be2b"
     "d672d8d2199ef37a59678f92443083e3b85edef8b45c71759371f823bab59a97126614f44"
     "d5001d45920180",
     "",
     1024,
     1,
     1,
     32,
     "01796dae1f78a72dfb09356db6f027cd884ba0201e6365b72aa54b3b00000000"},
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
        auto output256 = ot::Data::Factory();
        auto output512 = ot::Data::Factory();

        EXPECT_TRUE(crypto_.Hash().HMAC(
            ot::proto::HASHTYPE_SHA256,
            dataPassword->Bytes(),
            data->Bytes(),
            output256->WriteInto()));
        EXPECT_TRUE(crypto_.Hash().HMAC(
            ot::proto::HASHTYPE_SHA512,
            dataPassword->Bytes(),
            data->Bytes(),
            output512->WriteInto()));

        EXPECT_EQ(output256, expected256);
        EXPECT_EQ(output512, expected512);
    }
}

TEST_F(Test_Hash, scrypt_rfc7914)
{
    for (const auto& [input, salt, N, r, p, size, hex] : scrypt_rfc7914_) {
        const auto expected = ot::Data::Factory(hex, ot::Data::Mode::Hex);
        auto hash = ot::Data::Factory();
        const auto success = crypto_.Hash().Scrypt(
            input, salt, N, r, p, size, hash->WriteInto());

        EXPECT_TRUE(success);
        EXPECT_EQ(hash, expected);
    }
}

TEST_F(Test_Hash, scrypt_litecoin)
{
    for (const auto& [input, salt, N, r, p, size, hex] : scrypt_litecoin_) {
        const auto expected = ot::Data::Factory(hex, ot::Data::Mode::Hex);
        const auto preimage = ot::Data::Factory(input, ot::Data::Mode::Hex);
        auto hash = ot::Data::Factory();

        ASSERT_EQ(preimage->size(), 80);
        ASSERT_EQ(expected->size(), 32);

        const auto success = crypto_.Hash().Scrypt(
            preimage->Bytes(),
            preimage->Bytes(),
            N,
            r,
            p,
            size,
            hash->WriteInto());

        EXPECT_TRUE(success);
        EXPECT_EQ(hash, expected);
    }
}
}  // namespace
