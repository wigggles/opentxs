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
    static opentxs::OTPassword key_password_;
    static opentxs::proto::Ciphertext ciphertext_;
    static opentxs::proto::Ciphertext encrypted_password_;

    const opentxs::api::client::Manager& api_;
    std::shared_ptr<const opentxs::Nym> alice_;
    std::shared_ptr<const opentxs::Nym> bob_;

    Test_Symmetric()
        : api_(opentxs::OT::App().StartClient(args_, 0))
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
opentxs::OTPassword Test_Symmetric::key_password_{};
opentxs::proto::Ciphertext Test_Symmetric::ciphertext_{};
opentxs::proto::Ciphertext Test_Symmetric::encrypted_password_{};
}  // namespace

TEST_F(Test_Symmetric, create_key)
{
    ASSERT_TRUE(alice_);
    ASSERT_TRUE(bob_);

    opentxs::OTPasswordData password{""};

    ASSERT_TRUE(password.SetOverride(key_password_));

    key_ = api_.Crypto().Symmetric().Key(password, mode_);

    EXPECT_TRUE(key_.get());
}

TEST_F(Test_Symmetric, key_functionality)
{
    opentxs::OTPasswordData password{""};

    ASSERT_TRUE(password.SetOverride(key_password_));

    const auto encrypted = key_->Encrypt(
        TEST_PLAINTEXT,
        opentxs::Data::Factory(),
        password,
        ciphertext_,
        true,
        mode_);

    ASSERT_TRUE(encrypted);

    auto recoveredKey = api_.Crypto().Symmetric().Key(ciphertext_.key(), mode_);

    ASSERT_TRUE(recoveredKey.get());

    std::string plaintext{};
    auto decrypted = recoveredKey->Decrypt(ciphertext_, password, plaintext);

    ASSERT_TRUE(decrypted);
    EXPECT_STREQ(TEST_PLAINTEXT, plaintext.c_str());

    opentxs::OTPassword wrongPassword{};
    wrongPassword.setPassword("not the password");

    ASSERT_TRUE(password.SetOverride(wrongPassword));

    recoveredKey = api_.Crypto().Symmetric().Key(ciphertext_.key(), mode_);

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

    opentxs::OTPasswordData password{""};
    opentxs::OTPassword decryptedPassword{};
    auto recoveredKey = api_.Crypto().Symmetric().Key(ciphertext_.key(), mode_);

    ASSERT_TRUE(recoveredKey.get());

    const auto unlocked =
        alice_->Unlock(encrypted_password_, recoveredKey, decryptedPassword);

    ASSERT_TRUE(unlocked);
    ASSERT_TRUE(password.SetOverride(decryptedPassword));

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
    auto recoveredKey = api_.Crypto().Symmetric().Key(ciphertext_.key(), mode_);

    ASSERT_TRUE(recoveredKey.get());

    const auto unlocked =
        bob_->Unlock(encrypted_password_, recoveredKey, decryptedPassword);

    EXPECT_FALSE(unlocked);
}
