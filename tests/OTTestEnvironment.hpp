// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

#include "core/StateMachine.hpp"
#include "opentxs/crypto/library/Trezor.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/api/server/Server.hpp"
#if OT_BLOCKCHAIN
#include "blockchain/bitcoin/CompactSize.hpp"
#include "blockchain/p2p/bitcoin/message/Getblocks.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/Blockchain.hpp"
#endif  // OT_BLOCKCHAIN
#include "internal/identity/Identity.hpp"
#include "internal/otx/client/Client.hpp"
#include "server/Server.hpp"
#include "server/Transactor.hpp"
#include "Factory.hpp"

#include <gtest/gtest.h>

namespace ot = opentxs;

class OTTestEnvironment : public testing::Environment
{
public:
    static const ot::ArgList test_args_;

    virtual void SetUp();
    virtual void TearDown();

    virtual ~OTTestEnvironment();

private:
    static std::string random_path();
};
