// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include "Internal.hpp"
#include "opentxs/crypto/library/Bitcoin.hpp"
#include "opentxs/crypto/library/Trezor.hpp"
#include "Factory.hpp"

#include <gtest/gtest.h>

namespace ot = ::opentxs;

namespace
{
class Test_Bitcoin_Providers : public ::testing::Test
{
public:
    const ot::api::client::Manager& client_;
    const ot::api::Crypto& crypto_;
#if OT_CRYPTO_USING_TREZOR
    const std::unique_ptr<ot::crypto::Trezor> trezor_{
        ot::Factory::Trezor(crypto_)};
#endif
#if OT_CRYPTO_USING_LIBBITCOIN
    const std::unique_ptr<ot::crypto::Bitcoin> bitcoin_{
        ot::Factory::Bitcoin(crypto_)};
#endif
    const std::map<std::string, std::string> base_58_{
        {"", ""},
        {"00010966776006953D5567439E5E39F86A0D273BEE",
         "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM"},
    };

    Test_Bitcoin_Providers()
        : client_(ot::OT::App().StartClient({}, 0))
        , crypto_(client_.Crypto())
    {
    }

    bool test_base58_encode(const ot::crypto::Bip32& library)
    {
        for (const auto& [key, value] : base_58_) {
            auto input = ot::Data::Factory();
            input->DecodeHex(key);
            const auto output = crypto_.Encode().IdentifierEncode(input);

            EXPECT_EQ(value, output);
        }

        return true;
    }

    bool test_base58_decode(const ot::crypto::Bip32& library)
    {
        for (const auto& [key, value] : base_58_) {
            auto expected = ot::Data::Factory();
            expected->DecodeHex(key);
            const auto decoded = crypto_.Encode().IdentifierDecode(value);
            const auto output =
                ot::Data::Factory(decoded.c_str(), decoded.size());

            EXPECT_EQ(expected.get(), output.get());
        }

        return true;
    }

    bool test_ripemd160(const ot::crypto::Bip32& library)
    {
        // TODO test opentxs::api::crypto::Hash::Digest

        return true;
    }

    bool test_bip32_seed(const ot::crypto::Bip32& library)
    {
        // TODO test ot::crypto::Bip32::GetHDKey

        return true;
    }

    bool test_bip32_child_key(const ot::crypto::Bip32& library)
    {
        // TODO test ot::crypto::Bip32::GetChild

        return true;
    }

    bool test_bip32_seed_to_fingerprint(const ot::crypto::Bip32& library)
    {
        // TODO test ot::crypto::Bip32::SeedToFingerprint

        return true;
    }

    bool test_bip32_seed_to_key(const ot::crypto::Bip32& library)
    {
        // TODO test ot::crypto::Bip32::SeedToPrivateKey

        return true;
    }

    bool test_bip39_words(const ot::crypto::Bip32& library)
    {
        // TODO test ot::crypto::Bip39::SeedToWords

        return true;
    }

    bool test_bip39_seeds(const ot::crypto::Bip32& library)
    {
        // TODO test ot::crypto::Bip39::WordsToSeed

        return true;
    }
};

#if OT_CRYPTO_USING_TREZOR
TEST_F(Test_Bitcoin_Providers, Trezor)
{
    EXPECT_TRUE(test_base58_encode(*trezor_));
    EXPECT_TRUE(test_base58_decode(*trezor_));
    EXPECT_TRUE(test_ripemd160(*trezor_));
    EXPECT_TRUE(test_bip32_seed(*trezor_));
    EXPECT_TRUE(test_bip32_child_key(*trezor_));
    EXPECT_TRUE(test_bip32_seed_to_fingerprint(*trezor_));
    EXPECT_TRUE(test_bip32_seed_to_key(*trezor_));
    EXPECT_TRUE(test_bip39_words(*trezor_));
    EXPECT_TRUE(test_bip39_seeds(*trezor_));
}
#endif  // OT_CRYPTO_USING_TREZOR

#if OT_CRYPTO_USING_LIBBITCOIN
TEST_F(Test_Bitcoin_Providers, Libbitcoin)
{
    EXPECT_TRUE(test_base58_encode(*bitcoin_));
    EXPECT_TRUE(test_base58_decode(*bitcoin_));
    EXPECT_TRUE(test_ripemd160(*bitcoin_));
    EXPECT_TRUE(test_bip32_seed(*bitcoin_));
    EXPECT_TRUE(test_bip32_child_key(*bitcoin_));
    EXPECT_TRUE(test_bip32_seed_to_fingerprint(*bitcoin_));
    EXPECT_TRUE(test_bip32_seed_to_key(*bitcoin_));
    EXPECT_TRUE(test_bip39_words(*bitcoin_));
    EXPECT_TRUE(test_bip39_seeds(*bitcoin_));
}
#endif  // OT_CRYPTO_USING_LIBBITCOIN
}  // namespace
