// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_SIGNATURE_HPP
#define OPENTXS_CORE_CRYPTO_SIGNATURE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Armored.hpp"

namespace opentxs
{
class Signature : virtual public Armored
{
public:
    EXPORT static Pimpl<opentxs::Signature> Factory(const api::Core& api);

    EXPORT virtual const OTSignatureMetadata& getMetaData() const = 0;

    EXPORT virtual OTSignatureMetadata& getMetaData() = 0;

    EXPORT virtual ~Signature() = default;

protected:
    Signature() = default;

private:
    Signature(const Signature&) = delete;
    Signature(Signature&&) = delete;
    Signature& operator=(const Signature&) = delete;
    Signature& operator=(Signature&&) = delete;
};
}  // namespace opentxs
#endif
