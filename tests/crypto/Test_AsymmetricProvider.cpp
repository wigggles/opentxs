// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"
#include "Internal.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{
class Test_Signatures : public ::testing::Test
{
public:
    const opentxs::api::client::Manager& client_;
    const std::string fingerprint_;
    const proto::HashType hash_sha256_{proto::HASHTYPE_SHA256};
    const proto::HashType hash_sha512_{proto::HASHTYPE_SHA512};
    const proto::HashType hash_blake160_{proto::HASHTYPE_BLAKE2B160};
    const proto::HashType hash_blake256_{proto::HASHTYPE_BLAKE2B256};
    const proto::HashType hash_blake512_{proto::HASHTYPE_BLAKE2B512};
    const proto::HashType hash_ripemd160_{proto::HASHTYPE_RIMEMD160};
    const std::string plaintext_string_1_{"Test string"};
    const std::string plaintext_string_2_{"Another string"};
    const OTData plaintext_1{
        Data::Factory(plaintext_string_1_.data(), plaintext_string_1_.size())};
    const OTData plaintext_2{
        Data::Factory(plaintext_string_2_.data(), plaintext_string_2_.size())};
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    OTAsymmetricKey ed25519_hd_;
    const crypto::AsymmetricProvider& ed25519_;
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    OTAsymmetricKey secp256k1_hd_;
    const crypto::AsymmetricProvider& secp256k1_;
#endif
#if OT_CRYPTO_USING_LIBSECP256K1
#if OT_CRYPTO_USING_TREZOR
    const crypto::AsymmetricProvider& trezor_;
#endif  // OT_CRYPTO_USING_TREZOR
#endif  // OT_CRYPTO_USING_LIBSECP256K1

    Test_Signatures()
        : client_(opentxs::OT::App().StartClient({}, 0))
        , fingerprint_(client_.Exec().Wallet_ImportSeed(
              "response seminar brave tip suit recall often sound stick owner "
              "lottery motion",
              ""))
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        , ed25519_hd_(get_hd_key(client_, fingerprint_, EcdsaCurve::ED25519))
        , ed25519_(client_.Crypto().ED25519())
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        , secp256k1_hd_(
              get_hd_key(client_, fingerprint_, EcdsaCurve::SECP256K1))
        , secp256k1_(client_.Crypto().SECP256K1())
#endif
#if OT_CRYPTO_USING_LIBSECP256K1
#if OT_CRYPTO_USING_TREZOR
        , trezor_(dynamic_cast<const crypto::AsymmetricProvider&>(
              client_.Crypto().BIP32()))
#endif  // OT_CRYPTO_USING_TREZOR
#endif  // OT_CRYPTO_USING_LIBSECP256K1
    {
    }

    static OTAsymmetricKey get_hd_key(
        const opentxs::api::client::Manager& api,
        const std::string& fingerprint,
        const EcdsaCurve& curve)
    {
        std::string id{fingerprint};

        return OTAsymmetricKey{
            api.Seeds()
                .GetHDKey(
                    id,
                    curve,
                    {HDIndex{Bip43Purpose::NYM, Bip32Child::HARDENED},
                     HDIndex{0, Bip32Child::HARDENED},
                     HDIndex{0, Bip32Child::HARDENED},
                     HDIndex{0, Bip32Child::HARDENED},
                     HDIndex{Bip32Child::SIGN_KEY, Bip32Child::HARDENED}})
                .release()};
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

    bool bad_signature(
        const crypto::AsymmetricProvider& lib,
        const crypto::key::Asymmetric& key,
        const proto::HashType hash)
    {
        auto sig = Data::Factory();
        lib.Sign(plaintext_1, key, hash, sig);
        const auto verified = lib.Verify(plaintext_2, key, sig, hash);

        return !verified;
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
    EXPECT_EQ(true, bad_signature(ed25519_, ed25519_hd_, hash_blake256_));
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
    EXPECT_EQ(true, bad_signature(secp256k1_, ed25519_hd_, hash_blake256_));
    EXPECT_EQ(
        true,
        test_signature(plaintext_1, secp256k1_, secp256k1_hd_, hash_sha512_));
    EXPECT_EQ(true, bad_signature(secp256k1_, ed25519_hd_, hash_sha512_));
    EXPECT_EQ(
        true,
        test_signature(plaintext_1, secp256k1_, secp256k1_hd_, hash_blake160_));
    EXPECT_EQ(true, bad_signature(secp256k1_, ed25519_hd_, hash_blake160_));
    EXPECT_EQ(
        true,
        test_signature(plaintext_1, secp256k1_, secp256k1_hd_, hash_blake256_));
    EXPECT_EQ(true, bad_signature(secp256k1_, ed25519_hd_, hash_blake256_));
    EXPECT_EQ(
        true,
        test_signature(plaintext_1, secp256k1_, secp256k1_hd_, hash_blake512_));
    EXPECT_EQ(true, bad_signature(secp256k1_, ed25519_hd_, hash_blake512_));
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

#if OT_CRYPTO_USING_LIBSECP256K1
#if OT_CRYPTO_USING_TREZOR
TEST_F(Test_Signatures, Crosscheck_Trezor_Secp256k1)
{
    EXPECT_EQ(
        true,
        crosscheck_signature(
            plaintext_1, secp256k1_, trezor_, secp256k1_hd_, hash_sha256_));
    EXPECT_EQ(
        true,
        crosscheck_signature(
            plaintext_1, secp256k1_, trezor_, secp256k1_hd_, hash_sha512_));
    EXPECT_EQ(
        true,
        crosscheck_signature(
            plaintext_1, secp256k1_, trezor_, secp256k1_hd_, hash_blake160_));
    EXPECT_EQ(
        true,
        crosscheck_signature(
            plaintext_1, secp256k1_, trezor_, secp256k1_hd_, hash_blake256_));
    EXPECT_EQ(
        true,
        crosscheck_signature(
            plaintext_1, secp256k1_, trezor_, secp256k1_hd_, hash_blake512_));
    EXPECT_EQ(
        true,
        crosscheck_signature(
            plaintext_1, trezor_, secp256k1_, secp256k1_hd_, hash_sha256_));
    EXPECT_EQ(
        true,
        crosscheck_signature(
            plaintext_1, trezor_, secp256k1_, secp256k1_hd_, hash_sha512_));
    EXPECT_EQ(
        true,
        crosscheck_signature(
            plaintext_1, trezor_, secp256k1_, secp256k1_hd_, hash_blake160_));
    EXPECT_EQ(
        true,
        crosscheck_signature(
            plaintext_1, trezor_, secp256k1_, secp256k1_hd_, hash_blake256_));
    EXPECT_EQ(
        true,
        crosscheck_signature(
            plaintext_1, trezor_, secp256k1_, secp256k1_hd_, hash_blake512_));
}
#endif  // OT_CRYPTO_USING_TREZOR
#endif  // OT_CRYPTO_USING_LIBSECP256K1
}  // namespace
