// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/identity/Nym.hpp"

namespace opentxs::identity::internal
{
struct Nym : virtual public identity::Nym {
    enum class Mode : bool {
        Abbreviated = true,
        Full = false,
    };

    virtual Serialized SerializeCredentialIndex(const Mode mode) const = 0;
    virtual bool WriteCredentials() const = 0;

    virtual bool LoadCredentialIndex(const Serialized& index) = 0;
    virtual void SetAlias(const std::string& alias) = 0;
    virtual void SetAliasStartup(const std::string& alias) = 0;
};
}  // namespace opentxs::identity::internal
