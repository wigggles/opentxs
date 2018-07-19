// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_OTSIGNATURE_HPP
#define OPENTXS_CORE_CRYPTO_OTSIGNATURE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"

namespace opentxs
{

class String;

class OTSignature : public OTASCIIArmor
{
public:
    OTSignature()
        : OTASCIIArmor()
    {
    }

    virtual ~OTSignature() {}

    explicit OTSignature(const String& value)
        : OTASCIIArmor(value)
    {
    }

    explicit OTSignature(const OTASCIIArmor& value)
        : OTASCIIArmor(value)
    {
    }

    explicit OTSignature(const char* value)
        : OTASCIIArmor(value)
    {
    }

    OTSignatureMetadata& getMetaData() { return metadata_; }

    const OTSignatureMetadata& getMetaData() const { return metadata_; }

private:
    OTSignatureMetadata metadata_;
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_OTSIGNATURE_HPP
