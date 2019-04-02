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

#include "opentxs/opentxs.hpp"

#include "Internal.hpp"
#include "opentxs/crypto/library/Bitcoin.hpp"
#include "opentxs/crypto/library/Trezor.hpp"
#include "Factory.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{
class Test_Bitcoin_Providers : public ::testing::Test
{
public:
    const opentxs::api::client::Manager& client_;
    const api::Crypto& crypto_;
#if OT_CRYPTO_USING_TREZOR
    const std::unique_ptr<crypto::Trezor> trezor_{Factory::Trezor(crypto_)};
#endif
#if OT_CRYPTO_USING_LIBBITCOIN
    const std::unique_ptr<crypto::Bitcoin> bitcoin_{Factory::Bitcoin(crypto_)};
#endif

    Test_Bitcoin_Providers()
        : client_(opentxs::OT::App().StartClient({}, 0))
        , crypto_(client_.Crypto())
    {
    }

    bool test_base58_encode(const crypto::Bip32& library)
    {
        // TODO test opentxs::api::crypto::Encode::IdentifierEncode

        return true;
    }

    bool test_base58_decode(const crypto::Bip32& library)
    {
        // TODO test opentxs::api::crypto::Encode::IdentifierDecode

        return true;
    }

    bool test_ripemd160(const crypto::Bip32& library)
    {
        // TODO test opentxs::api::crypto::Hash::Digest

        return true;
    }

    bool test_bip32_seed(const crypto::Bip32& library)
    {
        // TODO test opentxs::crypto::Bip32::GetHDKey

        return true;
    }

    bool test_bip32_child_key(const crypto::Bip32& library)
    {
        // TODO test opentxs::crypto::Bip32::GetChild

        return true;
    }

    bool test_bip32_seed_to_fingerprint(const crypto::Bip32& library)
    {
        // TODO test opentxs::crypto::Bip32::SeedToFingerprint

        return true;
    }

    bool test_bip32_seed_to_key(const crypto::Bip32& library)
    {
        // TODO test opentxs::crypto::Bip32::SeedToPrivateKey

        return true;
    }

    bool test_bip39_words(const crypto::Bip32& library)
    {
        // TODO test opentxs::crypto::Bip39::SeedToWords

        return true;
    }

    bool test_bip39_seeds(const crypto::Bip32& library)
    {
        // TODO test opentxs::crypto::Bip39::WordsToSeed

        return true;
    }
};

#if OT_CRYPTO_USING_TREZOR
TEST_F(Test_Bitcoin_Providers, Trezor)
{
    EXPECT_EQ(true, test_base58_encode(*trezor_));
    EXPECT_EQ(true, test_base58_decode(*trezor_));
    EXPECT_EQ(true, test_ripemd160(*trezor_));
    EXPECT_EQ(true, test_bip32_seed(*trezor_));
    EXPECT_EQ(true, test_bip32_child_key(*trezor_));
    EXPECT_EQ(true, test_bip32_seed_to_fingerprint(*trezor_));
    EXPECT_EQ(true, test_bip32_seed_to_key(*trezor_));
    EXPECT_EQ(true, test_bip39_words(*trezor_));
    EXPECT_EQ(true, test_bip39_seeds(*trezor_));
}
#endif  // OT_CRYPTO_USING_TREZOR

#if OT_CRYPTO_USING_LIBBITCOIN
TEST_F(Test_Bitcoin_Providers, Libbitcoin)
{
    EXPECT_EQ(true, test_base58_encode(*bitcoin_));
    EXPECT_EQ(true, test_base58_decode(*bitcoin_));
    EXPECT_EQ(true, test_ripemd160(*bitcoin_));
    EXPECT_EQ(true, test_bip32_seed(*bitcoin_));
    EXPECT_EQ(true, test_bip32_child_key(*bitcoin_));
    EXPECT_EQ(true, test_bip32_seed_to_fingerprint(*bitcoin_));
    EXPECT_EQ(true, test_bip32_seed_to_key(*bitcoin_));
    EXPECT_EQ(true, test_bip39_words(*bitcoin_));
    EXPECT_EQ(true, test_bip39_seeds(*bitcoin_));
}
#endif  // OT_CRYPTO_USING_LIBBITCOIN
}  // namespace
