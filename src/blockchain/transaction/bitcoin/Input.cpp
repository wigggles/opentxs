// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Input.hpp"

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include <boost/endian/buffers.hpp>

#define OT_METHOD " opentxs::blockchain::transaction::bitcoin::Input::"

namespace be = boost::endian;

namespace opentxs::blockchain::transaction::bitcoin
{

OTData EncodeOutpoint(const Outpoint& in) noexcept
{
    try {
        OutpointHashField raw_hash{};
        OTPassword::safe_memcpy(
            &raw_hash, sizeof(raw_hash), in.first->data(), in.first->size());
        be::little_uint32_buf_t index(static_cast<std::uint32_t>(in.second));

        OutpointField raw_outpoint = std::make_pair(raw_hash, index);
        //        <OutpointHashField,
        //        be::little_uint32_buf_t>

        auto output = Data::Factory(&raw_outpoint, sizeof(raw_outpoint));

        return output;
    } catch (...) {
        return Data::Factory();
    }
}

bool DecodeOutpointFromPayload(
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    Outpoint& result) noexcept
{
    try {
        OutpointField raw_outpoint{};
        expectedSize += sizeof(raw_outpoint);
        if (expectedSize > size) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Outpoint field missing or incomplete")
                .Flush();

            return false;
        }
        OTPassword::safe_memcpy(
            &raw_outpoint, sizeof(raw_outpoint), it, sizeof(raw_outpoint));
        it += sizeof(raw_outpoint);
        // ----------------------------------------------------------
        result = std::make_pair  //<OTData, std::size_t>
            (Data::Factory(
                 raw_outpoint.first.data(), raw_outpoint.first.size()),
             raw_outpoint.second.value());
        // ----------------------------------------------------------
        return true;
    } catch (...) {
        return false;
    }
}

// ----------------------------------------------------------

Script Input::getScript() const
{
    auto* it{static_cast<const std::byte*>(signature_script_->data())};
    const std::size_t script_length = signature_script_->size();
    std::size_t expectedSize{};

    bitcoin::Script output{};
    const bool script_decoded = Output::DecodeScriptFromPayload(
        it, expectedSize, script_length, output);
    if (script_decoded) { return output; }

    return bitcoin::Script{};
}

Input::Sequence Input::getSequence() const { return sequence_; }

OTData Input::Encode() const noexcept
{
    try {
        auto output = EncodeOutpoint(previous_);

        const auto scriptLength =
            blockchain::bitcoin::CompactSize(signature_script_->size())
                .Encode();
        output->Concatenate(scriptLength.data(), scriptLength.size());

        output->Concatenate(
            signature_script_->data(), signature_script_->size());

        SequenceField raw_sequence(static_cast<std::uint32_t>(sequence_));
        output->Concatenate(&raw_sequence, sizeof(raw_sequence));

        return output;
    } catch (...) {
        return Data::Factory();
    }
}

bool Input::DecodeFromPayload(
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    Input& result) noexcept
{
    Outpoint previous_output{Data::Factory(), 0};

    const bool decodedOutpoint =
        DecodeOutpointFromPayload(it, expectedSize, size, previous_output);

    if (!decodedOutpoint) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed parsing outpoint").Flush();

        return false;
    }

    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Size below minimum for script length field")
            .Flush();

        return false;
    }
    // ----------------------------------------------------------
    auto scriptLength = std::size_t{0};
    const bool decodedScriptLength =
        blockchain::bitcoin::DecodeCompactSizeFromPayload(
            it, expectedSize, size, scriptLength);

    if (!decodedScriptLength) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": CompactSize incomplete for script length field")
            .Flush();

        return false;
    }
    if (scriptLength < 1) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Script length field cannot be zero")
            .Flush();

        return false;
    }
    // -----------------------------------------------
    expectedSize += scriptLength;

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Signature script field missing or incomplete")
            .Flush();

        return false;
    }

    const auto signature_script =
        Data::Factory(reinterpret_cast<const unsigned char*>(it), scriptLength);
    it += scriptLength;
    // -----------------------------------------------
    SequenceField raw_sequence{};

    expectedSize += sizeof(raw_sequence);

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Sequence field missing or incomplete")
            .Flush();

        return false;
    }
    // -----------------------------------------------
    OTPassword::safe_memcpy(
        &raw_sequence, sizeof(raw_sequence), it, sizeof(raw_sequence));
    it += sizeof(raw_sequence);

    const Sequence sequence = raw_sequence.value();
    // -----------------------------------------------
    result = Input(previous_output, signature_script, sequence);

    return true;
}

Input::Input()
    : previous_(std::make_pair(Data::Factory(), std::size_t(0)))
    , signature_script_(Data::Factory())
    , sequence_(0)
{
}

Input::Input(
    const Outpoint& outpoint,
    const Data& signature_script,
    const Sequence sequence) noexcept
    : previous_(outpoint)
    , signature_script_(Data::Factory(signature_script))
    , sequence_(sequence)
{
}

}  // namespace opentxs::blockchain::transaction::bitcoin
