// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: begin_exports
#include <opentxs/opentxs.hpp>
#include <gtest/gtest.h>
#include <string>

#include "1_Internal.hpp"
#include "2_Factory.hpp"
#include "core/StateMachine.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/api/server/Server.hpp"
#include "internal/identity/Identity.hpp"
#include "internal/otx/client/Client.hpp"
#include "server/Server.hpp"
#include "server/Transactor.hpp"
#if OT_BLOCKCHAIN
#include "blockchain/bitcoin/CompactSize.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "blockchain/p2p/bitcoin/message/Getblocks.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#endif  // OT_BLOCKCHAIN
// IWYU pragma: end_exports

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
