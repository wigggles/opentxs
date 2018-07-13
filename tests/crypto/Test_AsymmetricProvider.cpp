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

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{
class Test_Signatures : public ::testing::Test
{
public:
    const std::string fingerprint_{
        opentxs::OT::App().API().Exec().Wallet_ImportSeed(
            "response seminar brave tip suit recall often sound stick owner "
            "lottery motion",
            "")};
    const proto::HashType hash_sha256_{proto::HASHTYPE_SHA256};
    const proto::HashType hash_sha512_{proto::HASHTYPE_SHA512};
    const proto::HashType hash_blake160_{proto::HASHTYPE_BLAKE2B160};
    const proto::HashType hash_blake256_{proto::HASHTYPE_BLAKE2B256};
    const proto::HashType hash_blake512_{proto::HASHTYPE_BLAKE2B512};
    const proto::HashType hash_ripemd160_{proto::HASHTYPE_RIMEMD160};
    const std::string plaintext_string_1_{"Test string"};
    const OTData plaintext_1{
        Data::Factory(plaintext_string_1_.data(), plaintext_string_1_.size())};
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    OTAsymmetricKey ed25519_hd_{get_hd_key(fingerprint_, EcdsaCurve::ED25519)};
    const crypto::AsymmetricProvider& ed25519_{OT::App().Crypto().ED25519()};
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    OTAsymmetricKey secp256k1_hd_{
        get_hd_key(fingerprint_, EcdsaCurve::SECP256K1)};
    const crypto::AsymmetricProvider& secp256k1_{
        OT::App().Crypto().SECP256K1()};
#endif

    static OTAsymmetricKey get_hd_key(
        const std::string& fingerprint,
        const EcdsaCurve& curve)
    {
        std::string id{fingerprint};
        std::uint32_t notUsed{0};
        auto seed = OT::App().Crypto().BIP39().Seed(id, notUsed);
        proto::HDPath path{};
        path.set_version(1);
        path.set_root(id.c_str(), id.size());
        path.add_child(
            static_cast<std::uint32_t>(Bip43Purpose::NYM) |
            static_cast<std::uint32_t>(Bip32Child::HARDENED));
        path.add_child(0 | static_cast<std::uint32_t>(Bip32Child::HARDENED));
        path.add_child(0 | static_cast<std::uint32_t>(Bip32Child::HARDENED));
        path.add_child(0 | static_cast<std::uint32_t>(Bip32Child::HARDENED));
        path.add_child(
            static_cast<std::uint32_t>(Bip32Child::SIGN_KEY) |
            static_cast<std::uint32_t>(Bip32Child::HARDENED));
        const auto serialized =
            OT::App().Crypto().BIP32().GetHDKey(curve, *seed, path);

        return crypto::key::Asymmetric::Factory(*serialized);
    }

    bool test_signature(
        const Data& plaintext,
        const crypto::AsymmetricProvider& lib,
        const crypto::key::Asymmetric& key,
        const proto::HashType hash)
    {
        auto sig = Data::Factory();
        const auto haveSig = lib.Sign(plaintext, key, hash, sig);
        const auto verified = lib.Verify(plaintext, key, sig, hash);

        return haveSig || verified;
    }

    bool crosscheck_signature(
        const Data& plaintext,
        const crypto::AsymmetricProvider& signer,
        const crypto::AsymmetricProvider& verifier,
        const crypto::key::Asymmetric& key,
        const proto::HashType hash)
    {
        auto sig = Data::Factory();
        const auto haveSig = signer.Sign(plaintext, key, hash, sig);
        const auto verified = verifier.Verify(plaintext, key, sig, hash);

        return haveSig || verified;
    }
};

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
TEST_F(Test_Signatures, Ed25519_HD_Signatures)
{
    EXPECT_EQ(
        false,
        test_signature(plaintext_1, ed25519_, ed25519_hd_, hash_sha256_));
    EXPECT_EQ(
        false,
        test_signature(plaintext_1, ed25519_, ed25519_hd_, hash_sha512_));
    EXPECT_EQ(
        false,
        test_signature(plaintext_1, ed25519_, ed25519_hd_, hash_blake160_));
    EXPECT_EQ(
        true,
        test_signature(plaintext_1, ed25519_, ed25519_hd_, hash_blake256_));
    EXPECT_EQ(
        false,
        test_signature(plaintext_1, ed25519_, ed25519_hd_, hash_blake512_));
    EXPECT_EQ(
        false,
        test_signature(plaintext_1, ed25519_, ed25519_hd_, hash_ripemd160_));
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
TEST_F(Test_Signatures, Secp256k1_HD_Signatures)
{
    EXPECT_EQ(
        true,
        test_signature(plaintext_1, secp256k1_, secp256k1_hd_, hash_sha256_));
    EXPECT_EQ(
        true,
        test_signature(plaintext_1, secp256k1_, secp256k1_hd_, hash_sha512_));
    EXPECT_EQ(
        true,
        test_signature(plaintext_1, secp256k1_, secp256k1_hd_, hash_blake160_));
    EXPECT_EQ(
        true,
        test_signature(plaintext_1, secp256k1_, secp256k1_hd_, hash_blake256_));
    EXPECT_EQ(
        true,
        test_signature(plaintext_1, secp256k1_, secp256k1_hd_, hash_blake512_));
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
}  // namespace
