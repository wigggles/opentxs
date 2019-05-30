// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

#define TEST_MASTER_PASSWORD "test password"
#define TEST_PLAINTEXT "The quick brown fox jumped over the lazy dog."

namespace
{
const opentxs::ArgList args_{{{OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}}}};
bool init_{false};

struct Test_Symmetric : public ::testing::Test {
    static const opentxs::proto::SymmetricMode mode_;
    static opentxs::OTNymID alice_nym_id_;
    static opentxs::OTNymID bob_nym_id_;
    static opentxs::OTSymmetricKey key_;
    static opentxs::OTSymmetricKey second_key_;
    static opentxs::OTPassword key_password_;
    static opentxs::proto::Ciphertext ciphertext_;
    static opentxs::proto::Ciphertext second_ciphertext_;
    static opentxs::proto::Ciphertext encrypted_password_;
    static opentxs::proto::SessionKey session_key_;

    const opentxs::api::client::Manager& api_;
    opentxs::OTPasswordPrompt reason_;
    opentxs::Nym_p alice_;
    opentxs::Nym_p bob_;

    Test_Symmetric()
        : api_(opentxs::OT::App().StartClient(args_, 0))
        , reason_(api_.Factory().PasswordPrompt(__FUNCTION__))
    {
        if (false == init_) { init(); }

        alice_ = api_.Wallet().Nym(alice_nym_id_, reason_);
        bob_ = api_.Wallet().Nym(bob_nym_id_, reason_);
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
        const auto alice = api_.Exec().CreateNymHD(
            opentxs::proto::CITEMTYPE_INDIVIDUAL, "Alice", seedA, 0);
        const auto bob = api_.Exec().CreateNymHD(
            opentxs::proto::CITEMTYPE_INDIVIDUAL, "Bob", seedB, 0);
        alice_nym_id_->SetString(alice);
        bob_nym_id_->SetString(bob);
        key_password_.setPassword(TEST_MASTER_PASSWORD);
        init_ = true;
    }
};

const opentxs::proto::SymmetricMode Test_Symmetric::mode_{
    opentxs::proto::SMODE_CHACHA20POLY1305};
opentxs::OTNymID Test_Symmetric::alice_nym_id_{
    opentxs::identifier::Nym::Factory()};
opentxs::OTNymID Test_Symmetric::bob_nym_id_{
    opentxs::identifier::Nym::Factory()};
opentxs::OTSymmetricKey Test_Symmetric::key_{
    opentxs::crypto::key::Symmetric::Factory()};
opentxs::OTSymmetricKey Test_Symmetric::second_key_{
    opentxs::crypto::key::Symmetric::Factory()};
opentxs::OTPassword Test_Symmetric::key_password_{};
opentxs::proto::Ciphertext Test_Symmetric::ciphertext_{};
opentxs::proto::Ciphertext Test_Symmetric::second_ciphertext_{};
opentxs::proto::Ciphertext Test_Symmetric::encrypted_password_{};
opentxs::proto::SessionKey Test_Symmetric::session_key_{};
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

    const auto encrypted = key_->Encrypt(
        TEST_PLAINTEXT,
        opentxs::Data::Factory(),
        password,
        ciphertext_,
        true,
        mode_);

    ASSERT_TRUE(encrypted);

    auto recoveredKey = api_.Symmetric().Key(ciphertext_.key(), mode_);

    ASSERT_TRUE(recoveredKey.get());

    std::string plaintext{};
    auto decrypted = recoveredKey->Decrypt(ciphertext_, password, plaintext);

    ASSERT_TRUE(decrypted);
    EXPECT_STREQ(TEST_PLAINTEXT, plaintext.c_str());

    opentxs::OTPassword wrongPassword{};
    wrongPassword.setPassword("not the password");

    ASSERT_TRUE(password->SetPassword(wrongPassword));

    recoveredKey = api_.Symmetric().Key(ciphertext_.key(), mode_);

    ASSERT_TRUE(recoveredKey.get());

    decrypted = recoveredKey->Decrypt(ciphertext_, password, plaintext);

    EXPECT_FALSE(decrypted);
}

TEST_F(Test_Symmetric, lock)
{
    ASSERT_TRUE(alice_);

    const auto locked = alice_->Lock(key_password_, key_, encrypted_password_);

    ASSERT_TRUE(locked);
}

TEST_F(Test_Symmetric, unlock)
{
    ASSERT_TRUE(alice_);

    auto password = api_.Factory().PasswordPrompt("");
    opentxs::OTPassword decryptedPassword{};
    auto recoveredKey = api_.Symmetric().Key(ciphertext_.key(), mode_);

    ASSERT_TRUE(recoveredKey.get());

    const auto unlocked =
        alice_->Unlock(encrypted_password_, recoveredKey, decryptedPassword);

    ASSERT_TRUE(unlocked);
    ASSERT_TRUE(password->SetPassword(decryptedPassword));

    std::string plaintext{};
    const auto decrypted =
        recoveredKey->Decrypt(ciphertext_, password, plaintext);

    ASSERT_TRUE(decrypted);
    EXPECT_STREQ(TEST_PLAINTEXT, plaintext.c_str());
}

TEST_F(Test_Symmetric, wrongNym)
{
    ASSERT_TRUE(bob_);

    opentxs::OTPassword decryptedPassword{};
    auto recoveredKey = api_.Symmetric().Key(ciphertext_.key(), mode_);

    ASSERT_TRUE(recoveredKey.get());

    const auto unlocked =
        bob_->Unlock(encrypted_password_, recoveredKey, decryptedPassword);

    EXPECT_FALSE(unlocked);
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
        TEST_PLAINTEXT,
        opentxs::Data::Factory(),
        password,
        second_ciphertext_,
        true,
        mode_);

    ASSERT_TRUE(encrypted);
}

TEST_F(Test_Symmetric, seal)
{
    ASSERT_TRUE(bob_);

    const auto sealed =
        alice_->Seal(key_password_, second_key_, session_key_, reason_);

    ASSERT_TRUE(sealed);
}

TEST_F(Test_Symmetric, open)
{
    ASSERT_TRUE(bob_);

    auto password = api_.Factory().PasswordPrompt("");
    opentxs::OTPassword decryptedPassword{};
    auto recoveredKey = api_.Symmetric().Key(second_ciphertext_.key(), mode_);

    ASSERT_TRUE(recoveredKey.get());

    const auto unlocked =
        alice_->Open(session_key_, recoveredKey, decryptedPassword, reason_);

    ASSERT_TRUE(unlocked);
    ASSERT_TRUE(password->SetPassword(decryptedPassword));

    std::string plaintext{};
    const auto decrypted =
        recoveredKey->Decrypt(second_ciphertext_, password, plaintext);

    ASSERT_TRUE(decrypted);
    EXPECT_STREQ(TEST_PLAINTEXT, plaintext.c_str());
}
