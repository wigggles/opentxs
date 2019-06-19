// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/crypto/Bip32.hpp"

namespace opentxs::crypto::implementation
{
class Bip32 : virtual public opentxs::crypto::Bip32
{
public:
    bool DeserializePrivate(
        const std::string& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        OTPassword& key) const override;
    bool DeserializePublic(
        const std::string& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Data& key) const override;
    std::string SerializePrivate(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const OTPassword& key) const override;
    std::string SerializePublic(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const Data& key) const override;

protected:
    const api::Crypto& crypto_;

    Bip32(const api::Crypto& crypto);

private:
    OTData decode(const std::string& serialized) const;
    bool extract(
        const Data& input,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode) const;

    Bip32() = delete;
    Bip32(const Bip32&) = delete;
    Bip32(Bip32&&) = delete;
    Bip32& operator=(const Bip32&) = delete;
    Bip32& operator=(Bip32&&) = delete;
};
}  // namespace opentxs::crypto::implementation
