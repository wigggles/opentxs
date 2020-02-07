// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

#include <gtest/gtest.h>

namespace ot = opentxs;

class OTLowLevelTestEnvironment : public testing::Environment
{
public:
    static const ot::ArgList test_args_;

    virtual void SetUp();
    virtual void TearDown();

    virtual ~OTLowLevelTestEnvironment();

private:
    static std::string random_path();
};
