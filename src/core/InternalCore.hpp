// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef IMPLEMENTATION_OPENTXS_CORE_INTERNAL_HPP
#define IMPLEMENTATION_OPENTXS_CORE_INTERNAL_HPP

#include "Internal.hpp"

#include "opentxs/core/NymFile.hpp"

namespace opentxs::internal
{
struct NymFile : virtual public opentxs::NymFile {
    virtual bool LoadSignedNymFile() = 0;
    virtual bool SaveSignedNymFile() = 0;
};
}  // namespace opentxs::internal
#endif  // IMPLEMENTATION_OPENTXS_CORE_INTERNAL_HPP
