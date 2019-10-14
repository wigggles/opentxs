// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/util/Common.hpp"

#include <string>

std::string formatBool(bool in) { return in ? "true" : "false"; }

std::string formatTimestamp(const opentxs::Time in)
{
    return std::to_string(opentxs::Clock::to_time_t(in));
}

std::string getTimestamp() { return formatTimestamp(opentxs::Clock::now()); }

opentxs::Time parseTimestamp(std::string in)
{
    try {
        return opentxs::Clock::from_time_t(std::stoull(in));
    } catch (...) {

        return {};
    }
}
