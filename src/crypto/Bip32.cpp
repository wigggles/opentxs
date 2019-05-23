// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_WITH_BIP32
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip39.hpp"
#include "opentxs/OT.hpp"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include "Bip32.hpp"

#define OT_METHOD "opentxs::crypto::implementation::Bip32::"

namespace opentxs::crypto
{
std::string Print(const proto::HDPath& node)
{
    std::stringstream output{};
    output << node.root();

    for (const auto& child : node.child()) {
        output << " / ";
        const Bip32Index max = HDIndex{Bip32Child::HARDENED};

        if (max > child) {
            output << std::to_string(child);
        } else {
            output << std::to_string(child - max) << "'";
        }
    }

    return output.str();
}
}  // namespace opentxs::crypto

namespace opentxs::crypto::implementation
{
OTData Bip32::decode(const std::string& serialized) const
{
    auto input = OT::App().Crypto().Encode().IdentifierDecode(serialized);

    return Data::Factory(input.c_str(), input.size());
}

bool Bip32::DeserializePrivate(
    const std::string& serialized,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode,
    OTPassword& key) const
{
    const auto input = decode(serialized);
    const auto size = input->size();

    if (78 != size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid input size (")(size)(")")
            .Flush();

        return {};
    }

    bool output = extract(input, network, depth, parent, index, chainCode);

    if (std::byte(0) != input->at(45)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid padding bit").Flush();

        return {};
    }

    key.setMemory(&input->at(46), 32);

    return output;
}

bool Bip32::DeserializePublic(
    const std::string& serialized,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode,
    Data& key) const
{
    const auto input = decode(serialized);
    const auto size = input->size();

    if (78 != size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid input size (")(size)(")")
            .Flush();

        return {};
    }

    bool output = extract(input, network, depth, parent, index, chainCode);
    output &= input->Extract(33, key, 45);

    return output;
}

bool Bip32::extract(
    const Data& input,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode) const
{
    bool output{true};
    output &= input.Extract(network);
    output &= input.Extract(depth, 4);
    output &= input.Extract(parent, 5);
    output &= input.Extract(index, 9);
    output &= input.Extract(32, chainCode, 13);

    return output;
}

std::string Bip32::SerializePrivate(
    const Bip32Network network,
    const Bip32Depth depth,
    const Bip32Fingerprint parent,
    const Bip32Index index,
    const Data& chainCode,
    const OTPassword& key) const
{
    const auto size = key.getMemorySize();

    if (32 != size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid key size (")(size)(")")
            .Flush();

        return {};
    }

    auto input = Data::Factory();
    input->DecodeHex("0x00");

    OT_ASSERT(1 == input->size());

    input += Data::Factory(key.getMemory(), key.getMemorySize());

    OT_ASSERT(33 == input->size());

    return SerializePublic(network, depth, parent, index, chainCode, input);
}

std::string Bip32::SerializePublic(
    const Bip32Network network,
    const Bip32Depth depth,
    const Bip32Fingerprint parent,
    const Bip32Index index,
    const Data& chainCode,
    const Data& key) const
{
    auto size = key.size();

    if (33 != size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid key size (")(size)(")")
            .Flush();

        return {};
    }

    size = chainCode.size();

    if (32 != size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid chain code size (")(size)(
            ")")
            .Flush();

        return {};
    }

    auto output = Data::Factory(network);
    output.get() += depth;
    output.get() += parent;
    output.get() += index;
    output += chainCode;
    output += key;

    OT_ASSERT_MSG(78 == output->size(), std::to_string(output->size()).c_str());

    return OT::App().Crypto().Encode().IdentifierEncode(output);
}
}  // namespace opentxs::crypto::implementation
#endif
