// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>
#include <gtest/gtest.h>
#include <memory>

#include "Helpers.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/Blockchain.hpp"

namespace
{
TEST_F(Test_StartStop, init_opentxs) {}

TEST_F(Test_StartStop, ltc)
{
    EXPECT_FALSE(api_.Blockchain().Start(b::Type::Litecoin, "127.0.0.2"));
    EXPECT_FALSE(api_.Blockchain().Stop(b::Type::Litecoin));
}
}  // namespace
