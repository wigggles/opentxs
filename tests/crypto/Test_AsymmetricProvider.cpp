// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"

using namespace opentxs;

namespace
{
class Test_Signatures : public ::testing::Test
{
public:
    const ot::api::client::internal::Manager& client_;
    const std::string fingerprint_;
    const proto::HashType sha256_{proto::HASHTYPE_SHA256};
    const proto::HashType sha512_{proto::HASHTYPE_SHA512};
    const proto::HashType blake160_{proto::HASHTYPE_BLAKE2B160};
    const proto::HashType blake256_{proto::HASHTYPE_BLAKE2B256};
    const proto::HashType blake512_{proto::HASHTYPE_BLAKE2B512};
    const proto::HashType ripemd160_{proto::HASHTYPE_RIPEMD160};
    const std::string plaintext_string_1_{"Test string"};
    const std::string plaintext_string_2_{"Another string"};
    const OTData plaintext_1{
        Data::Factory(plaintext_string_1_.data(), plaintext_string_1_.size())};
    const OTData plaintext_2{
        Data::Factory(plaintext_string_2_.data(), plaintext_string_2_.size())};
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    OTAsymmetricKey ed_;
#if OT_CRYPTO_WITH_BIP32
    OTAsymmetricKey ed_hd_;
#endif  // OT_CRYPTO_WITH_BIP32
    const crypto::AsymmetricProvider& ed25519_;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    OTAsymmetricKey secp_;
#if OT_CRYPTO_WITH_BIP32
    OTAsymmetricKey secp_hd_;
#endif  // OT_CRYPTO_WITH_BIP32
    const crypto::AsymmetricProvider& secp256k1_;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    OTAsymmetricKey rsa_;
    const crypto::AsymmetricProvider& openssl_;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA

    [[maybe_unused]] Test_Signatures()
        : client_(dynamic_cast<const ot::api::client::internal::Manager&>(
              ot::Context().StartClient({}, 0)))
        , fingerprint_(client_.Exec().Wallet_ImportSeed(
              "response seminar brave tip suit recall often sound stick owner "
              "lottery motion",
              ""))
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        , ed_(get_key(client_, EcdsaCurve::ed25519))
#if OT_CRYPTO_WITH_BIP32
        , ed_hd_(get_hd_key(client_, fingerprint_, EcdsaCurve::ed25519))
#endif  // OT_CRYPTO_WITH_BIP32
        , ed25519_(client_.Crypto().ED25519())
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        , secp_(get_key(client_, EcdsaCurve::secp256k1))
#if OT_CRYPTO_WITH_BIP32
        , secp_hd_(get_hd_key(client_, fingerprint_, EcdsaCurve::secp256k1))
#endif  // OT_CRYPTO_WITH_BIP32
        , secp256k1_(client_.Crypto().SECP256K1())
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        , rsa_(get_key(client_, EcdsaCurve::invalid))
        , openssl_(client_.Crypto().RSA())
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
    {
    }

#if OT_CRYPTO_WITH_BIP32
    static OTAsymmetricKey get_hd_key(
        const ot::api::client::Manager& api,
        const std::string& fingerprint,
        const EcdsaCurve& curve)
    {
        auto reason = api.Factory().PasswordPrompt(__FUNCTION__);
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
                     HDIndex {
                         Bip32Child::SIGN_KEY,
                         Bip32Child::HARDENED
                     }},
                    reason)
                .release()};
    }
#endif  // OT_CRYPTO_WITH_BIP32
    [[maybe_unused]] static OTAsymmetricKey get_key(
        const ot::api::client::Manager& api,
        const EcdsaCurve curve)
    {
        const auto reason = api.Factory().PasswordPrompt(__FUNCTION__);

        if (EcdsaCurve::secp256k1 == curve) {
            auto params = NymParameters{NymParameterType::secp256k1};

            return api.Factory().AsymmetricKey(params, reason);
        } else if (EcdsaCurve::ed25519 == curve) {
            auto params = NymParameters{NymParameterType::ed25519};

            return api.Factory().AsymmetricKey(params, reason);
        } else {
#if OT_CRYPTO_SUPPORTED_KEY_RSA
            auto params = NymParameters{2048};

            return api.Factory().AsymmetricKey(params, reason);
#else
            OT_FAIL;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
        }
    }

    [[maybe_unused]] bool test_signature(
        const Data& plaintext,
        const crypto::AsymmetricProvider& lib,
        const crypto::key::Asymmetric& key,
        const proto::HashType hash)
    {
        auto reason = client_.Factory().PasswordPrompt(__FUNCTION__);
        auto sig = Data::Factory();
        const auto haveSig =
            lib.Sign(client_, plaintext, key, hash, sig, reason);

        if (false == haveSig) { return false; }

        const auto verified = lib.Verify(plaintext, key, sig, hash);

        return haveSig && verified;
    }

    [[maybe_unused]] bool bad_signature(
        const crypto::AsymmetricProvider& lib,
        const crypto::key::Asymmetric& key,
        const proto::HashType hash)
    {
        auto reason = client_.Factory().PasswordPrompt(__FUNCTION__);
        auto sig = Data::Factory();
        lib.Sign(client_, plaintext_1, key, hash, sig, reason);
        const auto verified = lib.Verify(plaintext_2, key, sig, hash);

        return !verified;
    }
};

#if OT_CRYPTO_SUPPORTED_KEY_RSA
TEST_F(Test_Signatures, RSA_unsupported_hash)
{
    EXPECT_FALSE(test_signature(plaintext_1, openssl_, rsa_, blake160_));
    EXPECT_FALSE(test_signature(plaintext_1, openssl_, rsa_, blake256_));
    EXPECT_FALSE(test_signature(plaintext_1, openssl_, rsa_, blake512_));
}

TEST_F(Test_Signatures, RSA_detect_invalid_signature)
{
    EXPECT_TRUE(bad_signature(openssl_, rsa_, sha256_));
    EXPECT_TRUE(bad_signature(openssl_, rsa_, sha512_));
    EXPECT_TRUE(bad_signature(openssl_, rsa_, ripemd160_));
}

TEST_F(Test_Signatures, RSA_supported_hashes)
{
    EXPECT_TRUE(test_signature(plaintext_1, openssl_, rsa_, sha256_));
    EXPECT_TRUE(test_signature(plaintext_1, openssl_, rsa_, sha512_));
    EXPECT_TRUE(test_signature(plaintext_1, openssl_, rsa_, ripemd160_));
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
TEST_F(Test_Signatures, Ed25519_unsupported_hash)
{
#if OT_CRYPTO_WITH_BIP32
    EXPECT_FALSE(test_signature(plaintext_1, ed25519_, ed_hd_, sha256_));
    EXPECT_FALSE(test_signature(plaintext_1, ed25519_, ed_hd_, sha512_));
    EXPECT_FALSE(test_signature(plaintext_1, ed25519_, ed_hd_, ripemd160_));
    EXPECT_FALSE(test_signature(plaintext_1, ed25519_, ed_hd_, blake160_));
    EXPECT_FALSE(test_signature(plaintext_1, ed25519_, ed_hd_, blake512_));
#endif  // OT_CRYPTO_WITH_BIP32

    EXPECT_FALSE(test_signature(plaintext_1, ed25519_, ed_, sha256_));
    EXPECT_FALSE(test_signature(plaintext_1, ed25519_, ed_, sha512_));
    EXPECT_FALSE(test_signature(plaintext_1, ed25519_, ed_, ripemd160_));
    EXPECT_FALSE(test_signature(plaintext_1, ed25519_, ed_, blake160_));
    EXPECT_FALSE(test_signature(plaintext_1, ed25519_, ed_, blake512_));
}

TEST_F(Test_Signatures, Ed25519_detect_invalid_signature)
{
#if OT_CRYPTO_WITH_BIP32
    EXPECT_TRUE(bad_signature(ed25519_, ed_hd_, blake256_));
#endif  // OT_CRYPTO_WITH_BIP32
    EXPECT_TRUE(bad_signature(ed25519_, ed_, blake256_));
}

TEST_F(Test_Signatures, Ed25519_supported_hashes)
{
#if OT_CRYPTO_WITH_BIP32
    EXPECT_TRUE(test_signature(plaintext_1, ed25519_, ed_hd_, blake256_));
#endif  // OT_CRYPTO_WITH_BIP32
    EXPECT_TRUE(test_signature(plaintext_1, ed25519_, ed_, blake256_));
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
TEST_F(Test_Signatures, Secp256k1_detect_invalid_signature)
{
#if OT_CRYPTO_WITH_BIP32
    EXPECT_TRUE(bad_signature(secp256k1_, secp_hd_, sha256_));
    EXPECT_TRUE(bad_signature(secp256k1_, secp_hd_, sha512_));
    EXPECT_TRUE(bad_signature(secp256k1_, secp_hd_, blake160_));
    EXPECT_TRUE(bad_signature(secp256k1_, secp_hd_, blake256_));
    EXPECT_TRUE(bad_signature(secp256k1_, secp_hd_, blake512_));
    EXPECT_TRUE(bad_signature(secp256k1_, secp_hd_, ripemd160_));
#endif  // OT_CRYPTO_WITH_BIP32
    EXPECT_TRUE(bad_signature(secp256k1_, secp_, sha256_));
    EXPECT_TRUE(bad_signature(secp256k1_, secp_, sha512_));
    EXPECT_TRUE(bad_signature(secp256k1_, secp_, blake160_));
    EXPECT_TRUE(bad_signature(secp256k1_, secp_, blake256_));
    EXPECT_TRUE(bad_signature(secp256k1_, secp_, blake512_));
    EXPECT_TRUE(bad_signature(secp256k1_, secp_, ripemd160_));
}

TEST_F(Test_Signatures, Secp256k1_supported_hashes)
{
#if OT_CRYPTO_WITH_BIP32
    EXPECT_TRUE(test_signature(plaintext_1, secp256k1_, secp_hd_, sha256_));
    EXPECT_TRUE(test_signature(plaintext_1, secp256k1_, secp_hd_, sha512_));
    EXPECT_TRUE(test_signature(plaintext_1, secp256k1_, secp_hd_, blake160_));
    EXPECT_TRUE(test_signature(plaintext_1, secp256k1_, secp_hd_, blake256_));
    EXPECT_TRUE(test_signature(plaintext_1, secp256k1_, secp_hd_, blake512_));
    EXPECT_TRUE(test_signature(plaintext_1, secp256k1_, secp_hd_, ripemd160_));
#endif  // OT_CRYPTO_WITH_BIP32

    EXPECT_TRUE(test_signature(plaintext_1, secp256k1_, secp_, sha256_));
    EXPECT_TRUE(test_signature(plaintext_1, secp256k1_, secp_, sha512_));
    EXPECT_TRUE(test_signature(plaintext_1, secp256k1_, secp_, blake256_));
    EXPECT_TRUE(test_signature(plaintext_1, secp256k1_, secp_, blake512_));
    EXPECT_TRUE(test_signature(plaintext_1, secp256k1_, secp_, blake160_));
    EXPECT_TRUE(test_signature(plaintext_1, secp256k1_, secp_, ripemd160_));
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
}  // namespace
