// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"

const opentxs::ArgList OTTestEnvironment::test_args_{
    {OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}}};

void OTTestEnvironment::SetUp() { opentxs::InitContext(test_args_); }

void OTTestEnvironment::TearDown() { opentxs::Cleanup(); }

OTTestEnvironment::~OTTestEnvironment() = default;
