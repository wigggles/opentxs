// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"

#define TEST_MASTER_PASSWORD "test password"
#define TEST_PLAINTEXT "The quick brown fox jumped over the lazy dog."

namespace
{
bool init_{false};

struct Test_Symmetric : public ::testing::Test {
    static const ot::proto::SymmetricMode mode_;
    static ot::OTNymID alice_nym_id_;
    static ot::OTNymID bob_nym_id_;
    static ot::OTSymmetricKey key_;
    static ot::OTSymmetricKey second_key_;
    static ot::OTPassword key_password_;
    static ot::proto::Ciphertext ciphertext_;
    static ot::proto::Ciphertext second_ciphertext_;
    static ot::proto::Ciphertext encrypted_password_;

    const ot::api::client::Manager& api_;
    ot::OTPasswordPrompt reason_;
    ot::Nym_p alice_;
    ot::Nym_p bob_;

    Test_Symmetric()
        : api_(ot::Context().StartClient(OTTestEnvironment::test_args_, 0))
        , reason_(api_.Factory().PasswordPrompt(__FUNCTION__))
        , alice_()
        , bob_()
    {
        if (false == init_) { init(); }

        alice_ = api_.Wallet().Nym(alice_nym_id_);
        bob_ = api_.Wallet().Nym(bob_nym_id_);
    }

    void init()
    {
        const auto seedA = api_.Exec().Wallet_ImportSeed(
            "spike nominee miss inquiry fee nothing belt list other "
            "daughter leave valley twelve gossip paper",
            "");
        const auto seedB = api_.Exec().Wallet_ImportSeed(
            "trim thunder unveil reduce crop cradle zone inquiry "
            "anchor skate property fringe obey butter text tank drama "
            "palm guilt pudding laundry stay axis prosper",
            "");
        alice_nym_id_ = api_.Wallet().Nym(reason_, "Alice", {seedA, 0})->ID();
        bob_nym_id_ = api_.Wallet().Nym(reason_, "Bob", {seedB, 0})->ID();
        key_password_.setPassword(TEST_MASTER_PASSWORD);
        init_ = true;
    }
};

const ot::proto::SymmetricMode Test_Symmetric::mode_{
    ot::proto::SMODE_CHACHA20POLY1305};
ot::OTNymID Test_Symmetric::alice_nym_id_{ot::identifier::Nym::Factory()};
ot::OTNymID Test_Symmetric::bob_nym_id_{ot::identifier::Nym::Factory()};
ot::OTSymmetricKey Test_Symmetric::key_{ot::crypto::key::Symmetric::Factory()};
ot::OTSymmetricKey Test_Symmetric::second_key_{
    ot::crypto::key::Symmetric::Factory()};
ot::OTPassword Test_Symmetric::key_password_{};
ot::proto::Ciphertext Test_Symmetric::ciphertext_{};
ot::proto::Ciphertext Test_Symmetric::second_ciphertext_{};
ot::proto::Ciphertext Test_Symmetric::encrypted_password_{};
}  // namespace

TEST_F(Test_Symmetric, create_key)
{
    ASSERT_TRUE(alice_);
    ASSERT_TRUE(bob_);

    auto password = api_.Factory().PasswordPrompt("");

    ASSERT_TRUE(password->SetPassword(key_password_));

    key_ = api_.Symmetric().Key(password, mode_);

    EXPECT_TRUE(key_.get());
}

TEST_F(Test_Symmetric, key_functionality)
{
    auto password = api_.Factory().PasswordPrompt("");

    ASSERT_TRUE(password->SetPassword(key_password_));

    const auto encrypted =
        key_->Encrypt(TEST_PLAINTEXT, password, ciphertext_, true, mode_);

    ASSERT_TRUE(encrypted);

    auto recoveredKey = api_.Symmetric().Key(ciphertext_.key(), mode_);

    ASSERT_TRUE(recoveredKey.get());

    std::string plaintext{};
    auto decrypted =
        recoveredKey->Decrypt(ciphertext_, password, [&](const auto size) {
            plaintext.resize(size);

            return ot::WritableView{plaintext.data(), plaintext.size()};
        });

    ASSERT_TRUE(decrypted);
    EXPECT_STREQ(TEST_PLAINTEXT, plaintext.c_str());

    ot::OTPassword wrongPassword{};
    wrongPassword.setPassword("not the password");

    ASSERT_TRUE(password->SetPassword(wrongPassword));

    recoveredKey = api_.Symmetric().Key(ciphertext_.key(), mode_);

    ASSERT_TRUE(recoveredKey.get());

    decrypted =
        recoveredKey->Decrypt(ciphertext_, password, [&](const auto size) {
            plaintext.resize(size);

            return ot::WritableView{plaintext.data(), plaintext.size()};
        });

    EXPECT_FALSE(decrypted);
}

TEST_F(Test_Symmetric, create_second_key)
{
    ASSERT_TRUE(alice_);
    ASSERT_TRUE(bob_);

    auto password = api_.Factory().PasswordPrompt("");

    ASSERT_TRUE(password->SetPassword(key_password_));

    second_key_ = api_.Symmetric().Key(password, mode_);

    EXPECT_TRUE(second_key_.get());

    const auto encrypted = second_key_->Encrypt(
        TEST_PLAINTEXT, password, second_ciphertext_, true, mode_);

    ASSERT_TRUE(encrypted);
}
