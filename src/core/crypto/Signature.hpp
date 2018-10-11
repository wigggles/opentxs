// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::implementation
{
class Signature final : virtual public opentxs::Signature, public Armored
{
public:
    const OTSignatureMetadata& getMetaData() const { return metadata_; }
    OTSignatureMetadata& getMetaData() { return metadata_; }

    Signature();

    ~Signature() = default;

private:
    friend OTSignature;

    OTSignatureMetadata metadata_;
};
}  // namespace opentxs::implementation
