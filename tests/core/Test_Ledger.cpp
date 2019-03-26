// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

namespace ot = opentxs;

const ot::ArgList args_{{{OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}}}};
ot::OTNymID nym_id_{ot::identifier::Nym::Factory()};
ot::OTServerID server_id_{ot::identifier::Server::Factory()};

namespace
{
struct Ledger : public ::testing::Test {
    const opentxs::api::client::Manager& client_;
    const opentxs::api::server::Manager& server_;

    Ledger()
        : client_(ot::OT::App().StartClient(args_, 0))
        , server_(ot::OT::App().StartServer(args_, 0, true))
    {
    }
};
}  // namespace

TEST_F(Ledger, init)
{
    const auto nymID =
        client_.Exec().CreateNymHD(ot::proto::CITEMTYPE_INDIVIDUAL, "Alice");

    ASSERT_FALSE(nymID.empty());

    nym_id_->SetString(nymID);
    const auto serverContract = server_.Wallet().Server(server_.ID());

    ASSERT_TRUE(serverContract);

    client_.Wallet().Server(serverContract->PublicContract());
    server_id_->SetString(serverContract->ID()->str());

    ASSERT_FALSE(server_id_->empty());
}

TEST_F(Ledger, create_nymbox)
{
    const auto nym = client_.Wallet().Nym(nym_id_);

    ASSERT_TRUE(nym);

    auto nymbox = client_.Factory().Ledger(
        nym_id_, nym_id_, server_id_, ot::ledgerType::nymbox, true);

    ASSERT_TRUE(nymbox);

    nymbox->ReleaseSignatures();

    EXPECT_TRUE(nymbox->SignContract(*nym));
    EXPECT_TRUE(nymbox->SaveContract());
    EXPECT_TRUE(nymbox->SaveNymbox());
}

TEST_F(Ledger, load_nymbox)
{
    auto nymbox = client_.Factory().Ledger(
        nym_id_, nym_id_, server_id_, ot::ledgerType::nymbox, false);

    ASSERT_TRUE(nymbox);
    EXPECT_TRUE(nymbox->LoadNymbox());
}
