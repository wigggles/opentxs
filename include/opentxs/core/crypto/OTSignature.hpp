// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_OTSIGNATURE_HPP
#define OPENTXS_CORE_CRYPTO_OTSIGNATURE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/Armored.hpp"

namespace opentxs
{

class String;

class OTSignature : public Armored
{
public:
    OTSignature()
        : Armored()
    {
    }

    virtual ~OTSignature() {}

    explicit OTSignature(const String& value)
        : Armored(value)
    {
    }

    explicit OTSignature(const Armored& value)
        : Armored(value)
    {
    }

    explicit OTSignature(const char* value)
        : Armored(value)
    {
    }

    OTSignatureMetadata& getMetaData() { return metadata_; }

    const OTSignatureMetadata& getMetaData() const { return metadata_; }

private:
    OTSignatureMetadata metadata_;
};

}  // namespace opentxs

#endif
