// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include "internal/identity/Identity.hpp"

#include <gtest/gtest.h>

namespace ot = opentxs;

namespace
{
class Test_Nym : public ::testing::Test
{
public:
    const ot::api::client::Manager& client_;
    const ot::OTPasswordPrompt reason_;

    Test_Nym()
        : client_(ot::Context().StartClient({}, 0))
        , reason_(client_.Factory().PasswordPrompt(__FUNCTION__))
    {
    }
};

TEST_F(Test_Nym, init_ot) {}

TEST_F(Test_Nym, storage)
{
    const auto alias = std::string{"alias"};
    std::unique_ptr<ot::identity::internal::Nym> pNym(ot::Factory::Nym(
        client_, {}, ot::proto::CITEMTYPE_INDIVIDUAL, alias, reason_));

    ASSERT_TRUE(pNym);

    auto& nym = *pNym;
    nym.SetAlias(alias);
    const auto id = ot::OTNymID{nym.ID()};

    EXPECT_TRUE(nym.VerifyPseudonym(reason_));

    {
        const auto serialized = nym.SerializeCredentialIndex(
            ot::identity::internal::Nym::Mode::Abbreviated);

        EXPECT_TRUE(ot::proto::Validate(serialized, ot::VERBOSE));
        EXPECT_TRUE(client_.Storage().Store(serialized, nym.Alias()));
    }

    {
        const auto nymList = client_.Storage().NymList();

        ASSERT_EQ(1, nymList.size());

        const auto& item = *nymList.begin();

        EXPECT_EQ(item.first, id->str());
        EXPECT_EQ(item.second, alias);
    }

    {
        auto pSerialized = std::shared_ptr<ot::proto::Nym>{};

        ASSERT_TRUE(client_.Storage().Load(id->str(), pSerialized));
        ASSERT_TRUE(pSerialized);

        const auto& serialized = *pSerialized;
        pNym.reset(ot::Factory::Nym(client_, serialized, alias, reason_));

        ASSERT_TRUE(pNym);

        const auto& loadedNym = *pNym;

        EXPECT_TRUE(loadedNym.CompareID(id));
        EXPECT_TRUE(loadedNym.VerifyPseudonym(reason_));
    }
}

TEST_F(Test_Nym, default_params)
{
    const auto pNym = client_.Wallet().Nym(reason_);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto& claims = nym.Claims();

    EXPECT_TRUE(nym.Alias().empty());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
    EXPECT_TRUE(nym.Name().empty());

    const auto pSection = claims.Section(ot::proto::CONTACTSECTION_SCOPE);

    EXPECT_FALSE(pSection);
}

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
TEST_F(Test_Nym, secp256k1_hd_bip47)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::secp256k1);
    params.setCredentialType(ot::proto::CREDTYPE_HD);
    params.SetSourceType(ot::proto::SOURCETYPE_BIP47);

    const auto pNym = client_.Wallet().Nym(reason_, "Nym", params);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto& claims = nym.Claims();

    EXPECT_EQ("Nym", nym.Alias());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
    EXPECT_EQ("Nym", nym.Name());

    const auto pSection = claims.Section(ot::proto::CONTACTSECTION_SCOPE);

    ASSERT_TRUE(pSection);

    const auto& section = *pSection;

    EXPECT_EQ(1, section.Size());

    const auto pGroup = section.Group(ot::proto::CITEMTYPE_INDIVIDUAL);

    ASSERT_TRUE(pGroup);

    const auto& group = *pGroup;
    const auto pItem = group.PrimaryClaim();

    ASSERT_TRUE(pItem);

    const auto& item = *pItem;

    EXPECT_EQ("Nym", item.Value());
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

#if OT_CRYPTO_SUPPORTED_KEY_HD
TEST_F(Test_Nym, secp256k1_hd_self_signed)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::secp256k1);
    params.setCredentialType(ot::proto::CREDTYPE_HD);
    params.SetSourceType(ot::proto::SOURCETYPE_PUBKEY);

    const auto pNym = client_.Wallet().Nym(reason_, "Nym", params);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto& claims = nym.Claims();

    EXPECT_EQ("Nym", nym.Alias());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
    EXPECT_EQ("Nym", nym.Name());

    const auto pSection = claims.Section(ot::proto::CONTACTSECTION_SCOPE);

    ASSERT_TRUE(pSection);

    const auto& section = *pSection;

    EXPECT_EQ(1, section.Size());

    const auto pGroup = section.Group(ot::proto::CITEMTYPE_INDIVIDUAL);

    ASSERT_TRUE(pGroup);

    const auto& group = *pGroup;
    const auto pItem = group.PrimaryClaim();

    ASSERT_TRUE(pItem);

    const auto& item = *pItem;

    EXPECT_EQ("Nym", item.Value());
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
TEST_F(Test_Nym, secp256k1_legacy_bip47)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::secp256k1);
    params.setCredentialType(ot::proto::CREDTYPE_LEGACY);
    params.SetSourceType(ot::proto::SOURCETYPE_BIP47);

    const auto pNym = client_.Wallet().Nym(reason_, "", params);

    EXPECT_FALSE(pNym);
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

TEST_F(Test_Nym, secp256k1_legacy_self_signed)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::secp256k1);
    params.setCredentialType(ot::proto::CREDTYPE_LEGACY);
    params.SetSourceType(ot::proto::SOURCETYPE_PUBKEY);

    const auto pNym = client_.Wallet().Nym(
        reason_, "Nym", params, ot::proto::CITEMTYPE_SERVER);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto& claims = nym.Claims();

    EXPECT_EQ("Nym", nym.Alias());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
    EXPECT_EQ("Nym", nym.Name());

    const auto pSection = claims.Section(ot::proto::CONTACTSECTION_SCOPE);

    ASSERT_TRUE(pSection);

    const auto& section = *pSection;

    EXPECT_EQ(1, section.Size());

    const auto pGroup = section.Group(ot::proto::CITEMTYPE_SERVER);

    ASSERT_TRUE(pGroup);

    const auto& group = *pGroup;
    const auto pItem = group.PrimaryClaim();

    ASSERT_TRUE(pItem);

    const auto& item = *pItem;

    EXPECT_EQ("Nym", item.Value());
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
TEST_F(Test_Nym, ed25519_hd_bip47)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::ed25519);
    params.setCredentialType(ot::proto::CREDTYPE_HD);
    params.SetSourceType(ot::proto::SOURCETYPE_BIP47);

    const auto pNym = client_.Wallet().Nym(reason_, "Nym", params);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto& claims = nym.Claims();

    EXPECT_EQ("Nym", nym.Alias());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
    EXPECT_EQ("Nym", nym.Name());

    const auto pSection = claims.Section(ot::proto::CONTACTSECTION_SCOPE);

    ASSERT_TRUE(pSection);

    const auto& section = *pSection;

    EXPECT_EQ(1, section.Size());

    const auto pGroup = section.Group(ot::proto::CITEMTYPE_INDIVIDUAL);

    ASSERT_TRUE(pGroup);

    const auto& group = *pGroup;
    const auto pItem = group.PrimaryClaim();

    ASSERT_TRUE(pItem);

    const auto& item = *pItem;

    EXPECT_EQ("Nym", item.Value());
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

#if OT_CRYPTO_SUPPORTED_KEY_HD
TEST_F(Test_Nym, ed25519_hd_self_signed)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::ed25519);
    params.setCredentialType(ot::proto::CREDTYPE_HD);
    params.SetSourceType(ot::proto::SOURCETYPE_PUBKEY);

    const auto pNym = client_.Wallet().Nym(reason_, "Nym", params);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto& claims = nym.Claims();

    EXPECT_EQ("Nym", nym.Alias());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
    EXPECT_EQ("Nym", nym.Name());

    const auto pSection = claims.Section(ot::proto::CONTACTSECTION_SCOPE);

    ASSERT_TRUE(pSection);

    const auto& section = *pSection;

    EXPECT_EQ(1, section.Size());

    const auto pGroup = section.Group(ot::proto::CITEMTYPE_INDIVIDUAL);

    ASSERT_TRUE(pGroup);

    const auto& group = *pGroup;
    const auto pItem = group.PrimaryClaim();

    ASSERT_TRUE(pItem);

    const auto& item = *pItem;

    EXPECT_EQ("Nym", item.Value());
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
TEST_F(Test_Nym, ed25519_legacy_bip47)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::ed25519);
    params.setCredentialType(ot::proto::CREDTYPE_LEGACY);
    params.SetSourceType(ot::proto::SOURCETYPE_BIP47);

    const auto pNym = client_.Wallet().Nym(reason_, "", params);

    EXPECT_FALSE(pNym);
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

TEST_F(Test_Nym, ed25519_legacy_self_signed)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::ed25519);
    params.setCredentialType(ot::proto::CREDTYPE_LEGACY);
    params.SetSourceType(ot::proto::SOURCETYPE_PUBKEY);

    const auto pNym = client_.Wallet().Nym(
        reason_, "Nym", params, ot::proto::CITEMTYPE_SERVER);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto& claims = nym.Claims();

    EXPECT_EQ("Nym", nym.Alias());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
    EXPECT_EQ("Nym", nym.Name());

    const auto pSection = claims.Section(ot::proto::CONTACTSECTION_SCOPE);

    ASSERT_TRUE(pSection);

    const auto& section = *pSection;

    EXPECT_EQ(1, section.Size());

    const auto pGroup = section.Group(ot::proto::CITEMTYPE_SERVER);

    ASSERT_TRUE(pGroup);

    const auto& group = *pGroup;
    const auto pItem = group.PrimaryClaim();

    ASSERT_TRUE(pItem);

    const auto& item = *pItem;

    EXPECT_EQ("Nym", item.Value());
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
}  // namespace
