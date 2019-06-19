// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/crypto/Signature.hpp"

#include "core/Armored.hpp"

#include "Signature.hpp"

// template class opentxs::Pimpl<opentxs::Signature>;

namespace opentxs
{
OTSignature Signature::Factory(const api::Core& api)
{
    return OTSignature(new implementation::Signature(api));
}
}  // namespace opentxs

namespace opentxs::implementation
{
Signature::Signature(const api::Core& api)
    : opentxs::Signature()
    , Armored()
    , metadata_(api)
{
}
}  // namespace opentxs::implementation
