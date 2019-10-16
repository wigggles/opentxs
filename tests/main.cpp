// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <opentxs/opentxs.hpp>
#include <gtest/gtest.h>

#include "OTTestEnvironment.hpp"

int main(int argc, char** argv)
{
    ::testing::AddGlobalTestEnvironment(new OTTestEnvironment());
    ::testing::InitGoogleTest(&argc, argv);

    std::string command("rm -r \"");
    command.append(opentxs::api::Context::Home());
    command.append("\"");
    [[maybe_unused]] const auto result{system(command.c_str())};

    return RUN_ALL_TESTS();
}
