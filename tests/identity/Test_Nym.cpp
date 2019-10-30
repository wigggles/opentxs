// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"

namespace ot = opentxs;

namespace
{
class Test_Nym : public ::testing::Test
{
public:
    const ot::api::client::internal::Manager& client_;
#if OT_STORAGE_FS
    const ot::api::client::internal::Manager& client_fs_;
#endif  // OT_STORAGE_FS
#if OT_STORAGE_SQLITE
    const ot::api::client::internal::Manager& client_sqlite_;
#endif  // OT_STORAGE_SQLITE
#if OT_STORAGE_LMDB
    const ot::api::client::internal::Manager& client_lmdb_;
#endif  // OT_STORAGE_LMDB
    const ot::OTPasswordPrompt reason_;

    bool test_storage(const ot::api::client::internal::Manager& api)
    {
        const auto reason = api.Factory().PasswordPrompt(__FUNCTION__);
        const auto alias = std::string{"alias"};
        std::unique_ptr<ot::identity::internal::Nym> pNym(ot::Factory::Nym(
            api, {}, ot::proto::CITEMTYPE_INDIVIDUAL, alias, reason));

        EXPECT_TRUE(pNym);

        if (!pNym) { return false; }

        auto& nym = *pNym;
        nym.SetAlias(alias);
        const auto id = ot::OTNymID{nym.ID()};

        EXPECT_TRUE(nym.VerifyPseudonym(reason));

        {
            const auto serialized = nym.SerializeCredentialIndex(
                ot::identity::internal::Nym::Mode::Abbreviated);

            EXPECT_TRUE(ot::proto::Validate(serialized, ot::VERBOSE));
            EXPECT_TRUE(api.Storage().Store(serialized, nym.Alias()));
        }

        {
            const auto nymList = api.Storage().NymList();

            EXPECT_EQ(1, nymList.size());

            if (1 > nymList.size()) { return false; }

            const auto& item = *nymList.begin();

            EXPECT_EQ(item.first, id->str());
            EXPECT_EQ(item.second, alias);
        }

        {
            auto pSerialized = std::shared_ptr<ot::proto::Nym>{};

            EXPECT_TRUE(api.Storage().Load(id->str(), pSerialized));
            EXPECT_TRUE(pSerialized);

            if (!pSerialized) { return false; }

            const auto& serialized = *pSerialized;
            pNym.reset(ot::Factory::Nym(api, serialized, alias, reason));

            EXPECT_TRUE(pNym);

            if (!pNym) { return false; }

            const auto& loadedNym = *pNym;

            EXPECT_TRUE(loadedNym.CompareID(id));
            EXPECT_TRUE(loadedNym.VerifyPseudonym(reason));
        }

        return true;
    }

    Test_Nym()
        : client_(dynamic_cast<const ot::api::client::internal::Manager&>(
              ot::Context().StartClient({}, 0)))
#if OT_STORAGE_FS
        , client_fs_(dynamic_cast<const ot::api::client::internal::Manager&>(
              ot::Context().StartClient(
                  {{OPENTXS_ARG_STORAGE_PLUGIN, {"fs"}}},
                  1)))
#endif  // OT_STORAGE_FS
#if OT_STORAGE_SQLITE
        , client_sqlite_(
              dynamic_cast<const ot::api::client::internal::Manager&>(
                  ot::Context().StartClient(
                      {{OPENTXS_ARG_STORAGE_PLUGIN, {"sqlite"}}},
                      2)))
#endif  // OT_STORAGE_SQLITE
#if OT_STORAGE_LMDB
        , client_lmdb_(dynamic_cast<const ot::api::client::internal::Manager&>(
              ot::Context().StartClient(
                  {{OPENTXS_ARG_STORAGE_PLUGIN, {"lmdb"}}},
                  3)))
#endif  // OT_STORAGE_LMDB
        , reason_(client_.Factory().PasswordPrompt(__FUNCTION__))
    {
    }
};

TEST_F(Test_Nym, init_ot) {}

TEST_F(Test_Nym, storage_memdb) { EXPECT_TRUE(test_storage(client_)); }

#if OT_STORAGE_FS
TEST_F(Test_Nym, storage_fs) { EXPECT_TRUE(test_storage(client_fs_)); }
#endif  // OT_STORAGE_FS
#if OT_STORAGE_SQLITE
TEST_F(Test_Nym, storage_sqlite) { EXPECT_TRUE(test_storage(client_sqlite_)); }
#endif  // OT_STORAGE_SQLITE
#if OT_STORAGE_LMDB
TEST_F(Test_Nym, storage_lmdb) { EXPECT_TRUE(test_storage(client_lmdb_)); }
#endif  // OT_STORAGE_LMDB

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
    EXPECT_EQ(ot::proto::SOURCETYPE_BIP47, nym.Source().Type());

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
    EXPECT_EQ(ot::proto::SOURCETYPE_PUBKEY, nym.Source().Type());

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
    EXPECT_EQ(ot::proto::SOURCETYPE_PUBKEY, nym.Source().Type());

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
    EXPECT_EQ(ot::proto::SOURCETYPE_BIP47, nym.Source().Type());

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
    EXPECT_EQ(ot::proto::SOURCETYPE_PUBKEY, nym.Source().Type());

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
    EXPECT_EQ(ot::proto::SOURCETYPE_PUBKEY, nym.Source().Type());

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
