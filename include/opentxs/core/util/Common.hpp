// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_UTIL_COMMON_HPP
#define OPENTXS_CORE_UTIL_COMMON_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Types.hpp"

#include <string>

std::string formatBool(bool in);
std::string formatTimestamp(const opentxs::Time in);
std::string getTimestamp();
opentxs::Time parseTimestamp(std::string in);
#endif
