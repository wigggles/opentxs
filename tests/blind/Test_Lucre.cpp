// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"
#include "Internal.hpp"
#include "Factory.hpp"

#include <gtest/gtest.h>

#define MINT_EXPIRE_MONTHS 6
#define MINT_VALID_MONTHS 12
#define REQUEST_PURSE_VALUE 20000

namespace
{
const opentxs::ArgList args_{{{OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}}}};
bool init_{false};

class Test_Basic : public ::testing::Test
{
public:
    static opentxs::OTNymID alice_nym_id_;
    static opentxs::OTNymID bob_nym_id_;
    static const opentxs::OTServerID server_id_;
    static const opentxs::OTUnitID unit_id_;
    static std::shared_ptr<opentxs::blind::Mint> mint_;
    static std::shared_ptr<opentxs::blind::Purse> request_purse_;
    static std::shared_ptr<opentxs::blind::Purse> issue_purse_;
    static opentxs::proto::Purse serialized_;
    static std::chrono::system_clock::time_point valid_from_;
    static std::chrono::system_clock::time_point valid_to_;

    const opentxs::api::client::Manager& api_;
    opentxs::OTPasswordPrompt reason_;
    opentxs::Nym_p alice_;
    opentxs::Nym_p bob_;

    Test_Basic()
        : api_(opentxs::Context().StartClient(args_, 0))
        , reason_(api_.Factory().PasswordPrompt(__FUNCTION__))
        , alice_()
        , bob_()
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
        alice_nym_id_ = api_.Wallet().Nym(reason_, "Alice", {seedA, 0})->ID();
        bob_nym_id_ = api_.Wallet().Nym(reason_, "Bob", {seedB, 0})->ID();
        const_cast<opentxs::identifier::UnitDefinition&>(unit_id_.get())
            .SetString(opentxs::Identifier::Random()->str());
        const_cast<opentxs::identifier::Server&>(server_id_.get())
            .SetString(opentxs::Identifier::Random()->str());
        init_ = true;
    }
};
}  // namespace

opentxs::OTNymID Test_Basic::alice_nym_id_{opentxs::identifier::Nym::Factory()};
opentxs::OTNymID Test_Basic::bob_nym_id_{opentxs::identifier::Nym::Factory()};
const opentxs::OTServerID Test_Basic::server_id_{
    opentxs::identifier::Server::Factory()};
const opentxs::OTUnitID Test_Basic::unit_id_{
    opentxs::identifier::UnitDefinition::Factory()};
std::shared_ptr<opentxs::blind::Mint> Test_Basic::mint_{};
std::shared_ptr<opentxs::blind::Purse> Test_Basic::request_purse_{};
std::shared_ptr<opentxs::blind::Purse> Test_Basic::issue_purse_{};
opentxs::proto::Purse Test_Basic::serialized_{};
std::chrono::system_clock::time_point Test_Basic::valid_from_;
std::chrono::system_clock::time_point Test_Basic::valid_to_;

TEST_F(Test_Basic, generateMint)
{
    mint_.reset(api_.Factory()
                    .Mint(
                        opentxs::String::Factory(bob_nym_id_->str()),
                        opentxs::String::Factory(unit_id_->str()))
                    .release());

    ASSERT_TRUE(mint_);

    const auto& nym = *bob_;
    const std::time_t now = std::time(nullptr);
    const std::chrono::seconds expireInterval(
        std::chrono::hours(MINT_EXPIRE_MONTHS * 30 * 24));
    const std::chrono::seconds validInterval(
        std::chrono::hours(MINT_VALID_MONTHS * 30 * 24));
    const std::time_t expires = now + expireInterval.count();
    const std::time_t validTo = now + validInterval.count();
    valid_from_ = std::chrono::system_clock::from_time_t(now);
    valid_to_ = std::chrono::system_clock::from_time_t(validTo);
    mint_->GenerateNewMint(
        api_.Wallet(),
        0,
        now,
        validTo,
        expires,
        unit_id_,
        server_id_,
        nym,
        1,
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000,
        1000000000,
        OT_MINT_KEY_SIZE_TEST,
        reason_);
}

TEST_F(Test_Basic, requestPurse)
{
    ASSERT_TRUE(alice_);
    ASSERT_TRUE(bob_);
    ASSERT_TRUE(mint_);

    request_purse_.reset(opentxs::Factory::Purse(
        api_,
        *alice_,
        server_id_,
        *bob_,
        opentxs::proto::CASHTYPE_LUCRE,
        *mint_,
        REQUEST_PURSE_VALUE,
        reason_));

    ASSERT_TRUE(request_purse_);

    auto& purse = *request_purse_;

    ASSERT_TRUE(purse.IsUnlocked());
    EXPECT_EQ(purse.EarliestValidTo(), valid_to_);
    EXPECT_EQ(purse.LatestValidFrom(), valid_from_);
    EXPECT_EQ(server_id_, purse.Notary());
    EXPECT_EQ(purse.State(), opentxs::proto::PURSETYPE_REQUEST);
    EXPECT_EQ(purse.Type(), opentxs::proto::CASHTYPE_LUCRE);
    EXPECT_EQ(unit_id_, purse.Unit());
    EXPECT_EQ(purse.Value(), REQUEST_PURSE_VALUE);
    ASSERT_EQ(purse.size(), 2);

    auto& token1 = purse.at(0);

    EXPECT_EQ(server_id_, token1.Notary());
    EXPECT_EQ(token1.Series(), 0);
    EXPECT_EQ(token1.State(), opentxs::proto::TOKENSTATE_BLINDED);
    EXPECT_EQ(token1.Type(), opentxs::proto::CASHTYPE_LUCRE);
    EXPECT_EQ(unit_id_, token1.Unit());
    EXPECT_EQ(token1.ValidFrom(), valid_from_);
    EXPECT_EQ(token1.ValidTo(), valid_to_);
    EXPECT_EQ(token1.Value(), 10000);

    auto& token2 = purse.at(1);

    EXPECT_EQ(server_id_, token2.Notary());
    EXPECT_EQ(token2.Series(), 0);
    EXPECT_EQ(token2.State(), opentxs::proto::TOKENSTATE_BLINDED);
    EXPECT_EQ(token2.Type(), opentxs::proto::CASHTYPE_LUCRE);
    EXPECT_EQ(unit_id_, token2.Unit());
    EXPECT_EQ(token2.ValidFrom(), valid_from_);
    EXPECT_EQ(token2.ValidTo(), valid_to_);
    EXPECT_EQ(token2.Value(), 10000);
}

TEST_F(Test_Basic, serialize)
{
    ASSERT_TRUE(request_purse_);

    serialized_ = request_purse_->Serialize();

    EXPECT_TRUE(opentxs::proto::Validate(serialized_, false));
    EXPECT_EQ(serialized_.version(), 1);
    EXPECT_EQ(serialized_.type(), opentxs::proto::CASHTYPE_LUCRE);
    EXPECT_EQ(serialized_.state(), opentxs::proto::PURSETYPE_REQUEST);
    EXPECT_EQ(serialized_.notary(), server_id_->str());
    EXPECT_EQ(serialized_.mint(), unit_id_->str());
    EXPECT_EQ(serialized_.totalvalue(), REQUEST_PURSE_VALUE);
    EXPECT_EQ(
        serialized_.latestvalidfrom(),
        std::chrono::system_clock::to_time_t(valid_from_));
    EXPECT_EQ(
        serialized_.earliestvalidto(),
        std::chrono::system_clock::to_time_t(valid_to_));
    ASSERT_EQ(serialized_.token_size(), 2);

    auto& token1 = serialized_.token(0);

    EXPECT_EQ(token1.version(), 1);
    EXPECT_EQ(token1.type(), opentxs::proto::CASHTYPE_LUCRE);
    EXPECT_EQ(token1.state(), opentxs::proto::TOKENSTATE_BLINDED);
    EXPECT_EQ(token1.notary(), server_id_->str());
    EXPECT_EQ(token1.mint(), unit_id_->str());
    EXPECT_EQ(token1.series(), 0);
    EXPECT_EQ(token1.denomination(), 10000);
    EXPECT_EQ(
        token1.validfrom(), std::chrono::system_clock::to_time_t(valid_from_));
    EXPECT_EQ(
        token1.validto(), std::chrono::system_clock::to_time_t(valid_to_));

    auto& token2 = serialized_.token(1);

    EXPECT_EQ(token2.version(), 1);
    EXPECT_EQ(token2.type(), opentxs::proto::CASHTYPE_LUCRE);
    EXPECT_EQ(token2.state(), opentxs::proto::TOKENSTATE_BLINDED);
    EXPECT_EQ(token2.notary(), server_id_->str());
    EXPECT_EQ(token2.mint(), unit_id_->str());
    EXPECT_EQ(token2.series(), 0);
    EXPECT_EQ(token2.denomination(), 10000);
    EXPECT_EQ(
        token2.validfrom(), std::chrono::system_clock::to_time_t(valid_from_));
    EXPECT_EQ(
        token2.validto(), std::chrono::system_clock::to_time_t(valid_to_));
}

TEST_F(Test_Basic, deserialize)
{
    std::unique_ptr<opentxs::blind::Purse> restored{
        opentxs::Factory::Purse(api_, serialized_)};

    ASSERT_TRUE(restored);

    auto& purse = *restored;

    EXPECT_FALSE(purse.IsUnlocked());
    EXPECT_EQ(purse.EarliestValidTo(), valid_to_);
    EXPECT_EQ(purse.LatestValidFrom(), valid_from_);
    EXPECT_EQ(server_id_, purse.Notary());
    EXPECT_EQ(purse.State(), opentxs::proto::PURSETYPE_REQUEST);
    EXPECT_EQ(purse.Type(), opentxs::proto::CASHTYPE_LUCRE);
    EXPECT_EQ(unit_id_, purse.Unit());
    EXPECT_EQ(purse.Value(), REQUEST_PURSE_VALUE);
    ASSERT_EQ(purse.size(), 2);

    auto& token1 = purse.at(0);

    EXPECT_EQ(server_id_, token1.Notary());
    EXPECT_EQ(token1.Series(), 0);
    EXPECT_EQ(token1.State(), opentxs::proto::TOKENSTATE_BLINDED);
    EXPECT_EQ(token1.Type(), opentxs::proto::CASHTYPE_LUCRE);
    EXPECT_EQ(unit_id_, token1.Unit());
    EXPECT_EQ(token1.ValidFrom(), valid_from_);
    EXPECT_EQ(token1.ValidTo(), valid_to_);
    EXPECT_EQ(token1.Value(), 10000);

    auto& token2 = purse.at(1);

    EXPECT_EQ(server_id_, token2.Notary());
    EXPECT_EQ(token2.Series(), 0);
    EXPECT_EQ(token2.State(), opentxs::proto::TOKENSTATE_BLINDED);
    EXPECT_EQ(token2.Type(), opentxs::proto::CASHTYPE_LUCRE);
    EXPECT_EQ(unit_id_, token2.Unit());
    EXPECT_EQ(token2.ValidFrom(), valid_from_);
    EXPECT_EQ(token2.ValidTo(), valid_to_);
    EXPECT_EQ(token2.Value(), 10000);
}

TEST_F(Test_Basic, sign)
{
    std::unique_ptr<opentxs::blind::Purse> restored{
        opentxs::Factory::Purse(api_, serialized_)};

    ASSERT_TRUE(restored);

    ASSERT_TRUE(mint_);
    ASSERT_TRUE(alice_);
    ASSERT_TRUE(bob_);

    auto& requestPurse = *restored;
    auto& alice = *alice_;
    auto& bob = *bob_;

    EXPECT_TRUE(requestPurse.Unlock(bob, reason_));
    ASSERT_TRUE(requestPurse.IsUnlocked());

    issue_purse_.reset(
        opentxs::Factory::Purse(api_, requestPurse, alice, reason_));

    ASSERT_TRUE(issue_purse_);

    auto& issuePurse = *issue_purse_;

    EXPECT_TRUE(issuePurse.IsUnlocked());

    auto& mint = *mint_;
    auto pToken = requestPurse.Pop();
    const auto added = issuePurse.AddNym(bob, reason_);

    EXPECT_TRUE(added);

    while (pToken) {
        auto& token = *pToken;
        const auto signature = mint.SignToken(bob, token, reason_);

        EXPECT_TRUE(signature);
        EXPECT_TRUE(opentxs::proto::TOKENSTATE_SIGNED == token.State());

        const auto push = issuePurse.Push(pToken, reason_);

        EXPECT_TRUE(push);

        pToken = requestPurse.Pop();
    }

    EXPECT_TRUE(opentxs::proto::PURSETYPE_ISSUE == issuePurse.State());
    EXPECT_EQ(issuePurse.Notary().str(), requestPurse.Notary().str());
    EXPECT_EQ(issuePurse.Type(), requestPurse.Type());
    EXPECT_EQ(issuePurse.Unit().str(), requestPurse.Unit().str());
    EXPECT_EQ(issuePurse.Value(), REQUEST_PURSE_VALUE);
    EXPECT_EQ(requestPurse.Value(), 0);
}

TEST_F(Test_Basic, process)
{
    ASSERT_TRUE(issue_purse_);
    ASSERT_TRUE(mint_);
    ASSERT_TRUE(alice_);

    auto& issuePurse = *issue_purse_;
    std::unique_ptr<opentxs::blind::Purse> restored{
        opentxs::Factory::Purse(api_, issuePurse.Serialize())};

    ASSERT_TRUE(restored);

    auto& purse = *restored;
    auto& mint = *mint_;
    auto& alice = *alice_;

    EXPECT_TRUE(purse.Unlock(alice, reason_));
    ASSERT_TRUE(purse.IsUnlocked());
    EXPECT_TRUE(purse.Process(alice, mint, reason_));
    EXPECT_TRUE(opentxs::proto::PURSETYPE_NORMAL == purse.State());

    issue_purse_ = std::move(restored);
}

TEST_F(Test_Basic, verify)
{
    ASSERT_TRUE(issue_purse_);
    ASSERT_TRUE(mint_);
    ASSERT_TRUE(bob_);

    auto& issuePurse = *issue_purse_;
    auto& bob = *bob_;

    EXPECT_TRUE(issuePurse.Unlock(bob, reason_));
    ASSERT_TRUE(issuePurse.IsUnlocked());

    std::unique_ptr<opentxs::blind::Purse> restored{
        opentxs::Factory::Purse(api_, issuePurse.Serialize())};

    ASSERT_TRUE(restored);

    auto& purse = *restored;

    EXPECT_TRUE(purse.Unlock(bob, reason_));
    ASSERT_TRUE(purse.IsUnlocked());

    auto& mint = *mint_;

    for (const auto& token : purse) {
        EXPECT_FALSE(token.IsSpent(reason_));

        const auto verified = mint.VerifyToken(bob, token, reason_);

        EXPECT_TRUE(verified);
    }

    issue_purse_ = std::move(restored);
}

TEST_F(Test_Basic, wallet)
{
    {
        auto purse =
            api_.Wallet().Purse(alice_nym_id_, server_id_, unit_id_, true);

        EXPECT_FALSE(purse);
    }

    {
        api_.Wallet().mutable_Purse(
            alice_nym_id_,
            server_id_,
            unit_id_,
            reason_,
            opentxs::proto::CASHTYPE_LUCRE);
    }

    {
        auto purse =
            api_.Wallet().Purse(alice_nym_id_, server_id_, unit_id_, false);

        EXPECT_TRUE(purse);
    }
}

TEST_F(Test_Basic, PushPop)
{
    auto purseEditor = api_.Wallet().mutable_Purse(
        alice_nym_id_,
        server_id_,
        unit_id_,
        reason_,
        opentxs::proto::CASHTYPE_LUCRE);
    auto& purse = purseEditor.get();

    ASSERT_TRUE(issue_purse_);

    auto& issuePurse = *issue_purse_;
    auto& alice = *alice_;
    const auto unlocked = issuePurse.Unlock(alice, reason_);

    ASSERT_TRUE(unlocked);
    ASSERT_TRUE(purse.Unlock(alice, reason_));
    ASSERT_TRUE(purse.IsUnlocked());

    auto token = issuePurse.Pop();

    while (token) {
        EXPECT_TRUE(token->MarkSpent(reason_));
        EXPECT_TRUE(token->IsSpent(reason_));
        EXPECT_TRUE(purse.Push(token, reason_));

        token = issuePurse.Pop();
    }

    EXPECT_EQ(purse.size(), 2);
    EXPECT_EQ(purse.Value(), 0);
    EXPECT_EQ(issuePurse.Value(), 0);
}
