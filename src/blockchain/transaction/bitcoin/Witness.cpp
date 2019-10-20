// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"

#include "internal/blockchain/bitcoin/Bitcoin.hpp"

#include "Witness.hpp"

#define OT_METHOD " opentxs::blockchain::transaction::bitcoin::Witness::"

namespace opentxs::blockchain::transaction::bitcoin
{
Witness::Witness(const std::vector<OTData>& components) noexcept
    : components_(components)
{
}

Witness::Witness() noexcept
    : components_()
{
}

OTData Witness::Encode() const noexcept
{
    try {
        const auto count =
            blockchain::bitcoin::CompactSize(components_.size()).Encode();
        auto output = Data::Factory(count.data(), count.size());

        for (const auto& component : components_) {
            const auto size =
                blockchain::bitcoin::CompactSize(component->size()).Encode();
            output->Concatenate(size.data(), size.size());

            output->Concatenate(component->data(), component->size());
        }

        return output;
    } catch (...) {
        return Data::Factory();
    }
}

// The TxWitness structure consists of a var_int count of witness data
// components, followed by (for each witness data component) a var_int length of
// the component and the raw component data itself.
bool Witness::DecodeFromPayload(
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    Witness& txWitness) noexcept
{
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Size below minimum for TxWitness component count")
            .Flush();

        return false;
    }
    // ----------------------------------------------------------
    auto componentCount = std::size_t{0};
    const bool decodedComponentCount =
        blockchain::bitcoin::DecodeCompactSizeFromPayload(
            it, expectedSize, size, componentCount);

    if (!decodedComponentCount) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": CompactSize incomplete for TxWitness component count field")
            .Flush();

        return false;
    }
    if (componentCount < 1) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": TxWitness component count field cannot be zero")
            .Flush();

        return false;
    }
    // -----------------------------------------------
    std::vector<OTData> components;

    for (std::size_t ii = 0; ii < componentCount; ii++) {
        expectedSize += sizeof(std::byte);

        if (expectedSize > size) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Size below minimum for TxWitness component length")
                .Flush();

            return false;
        }
        // ----------------------------------------------------------
        auto componentLength = std::size_t{0};
        const bool decodedComponentLength =
            blockchain::bitcoin::DecodeCompactSizeFromPayload(
                it, expectedSize, size, componentLength);

        if (!decodedComponentLength) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": CompactSize incomplete for TxWitness component length field")
                .Flush();

            return false;
        }
        if (componentLength < 1) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": TxWitness component length field cannot be zero")
                .Flush();

            return false;
        }
        // -----------------------------------------------
        expectedSize += componentLength;

        if (expectedSize > size) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": TxWitness component field missing or incomplete")
                .Flush();

            return false;
        }

        const auto witness_component = Data::Factory(
            reinterpret_cast<const unsigned char*>(it), componentLength);
        it += componentLength;

        components.push_back(witness_component);
    }
    // -----------------------------------------------
    txWitness = Witness(components);

    return true;
}
}  // namespace opentxs::blockchain::transaction::bitcoin
