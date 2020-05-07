// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Types.hpp"

namespace opentxs
{
class Cleanup
{
public:
    Cleanup(SimpleCallback cb) noexcept;
    ~Cleanup();

private:
    const SimpleCallback cb_;
};
}  // namespace opentxs
