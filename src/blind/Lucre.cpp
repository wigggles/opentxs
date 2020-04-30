// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#if OT_CASH_USING_LUCRE
#include "blind/Lucre.hpp"  // IWYU pragma: associated

#include <cstdio>

#include "opentxs/Version.hpp"

namespace opentxs::blind
{
LucreDumper::LucreDumper()
    : m_str_dumpfile()
{
    SetDumper(stderr);
}
}  // namespace opentxs::blind
#endif
