// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/NymFile.hpp"

namespace opentxs
{
template <>
struct make_blank<OTData> {
    static OTData value() { return Data::Factory(); }
};
template <>
struct make_blank<OTIdentifier> {
    static OTIdentifier value() { return Identifier::Factory(); }
};
}  // namespace opentxs

namespace opentxs::internal
{
struct NymFile : virtual public opentxs::NymFile {
    virtual bool LoadSignedNymFile(const PasswordPrompt& reason) = 0;
    virtual bool SaveSignedNymFile(const PasswordPrompt& reason) = 0;
};
}  // namespace opentxs::internal
