// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"

#include <boost/filesystem.hpp>

const opentxs::ArgList OTTestEnvironment::test_args_{
    {OPENTXS_ARG_HOME, {OTTestEnvironment::random_path()}},
    {OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}},
};

namespace fs = boost::filesystem;

std::string OTTestEnvironment::random_path()
{
    const auto path = fs::temp_directory_path() /
                      fs::unique_path("opentxs-test-%%%%-%%%%-%%%%-%%%%");

    assert(fs::create_directories(path));

    return path.string();
}

void OTTestEnvironment::SetUp() { opentxs::InitContext(test_args_); }

void OTTestEnvironment::TearDown() { opentxs::Cleanup(); }

OTTestEnvironment::~OTTestEnvironment() = default;
