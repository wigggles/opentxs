// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CASH_USING_LUCRE

#include <cstdio>

#include "Lucre.hpp"

namespace opentxs::blind
{
LucreDumper::LucreDumper()
    : m_str_dumpfile()
{
    SetDumper(stderr);
}
}  // namespace opentxs::blind
#endif
