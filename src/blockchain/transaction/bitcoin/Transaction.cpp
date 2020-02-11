// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Transaction.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include "internal/blockchain/bitcoin/Bitcoin.hpp"

#include <boost/endian/buffers.hpp>

#define OT_METHOD "opentxs::blockchain::transaction::bitcoin::Transaction::"

namespace be = boost::endian;

namespace opentxs::blockchain::transaction::bitcoin
{
Transaction::Transaction(
    const api::internal::Core& api,
    const blockchain::Type network,
    const TxDataFormatVersion data_format_version,
    const bool witness_flag_present,
    const Inputs& inputs,
    const Outputs& outputs,
    const Witnesses& witnesses,
    const TxLockTime lock_time) noexcept
    : api_(api)
    , network_(network)
    , data_format_version_(data_format_version)
    , witness_flag_present_(witness_flag_present)
    , inputs_(inputs)
    , outputs_(outputs)
    , witnesses_(witnesses)
    , lock_time_(lock_time)
{
}

OTData Transaction::Encode() const noexcept
{
    try {
        TxDataFormatVersionField versionField(
            static_cast<TxDataFormatVersion>(data_format_version_));

        auto output = Data::Factory(&versionField, sizeof(versionField));

        if (witness_flag_present_) {
            WitnessFlagField raw_witness_flag(
                {be::little_uint8_buf_t(std::uint8_t(0)),
                 be::little_uint8_buf_t(std::uint8_t(1))});
            output->Concatenate(&raw_witness_flag, sizeof(raw_witness_flag));
        }
        // ------------------------------------------------
        const auto inputCount =
            blockchain::bitcoin::CompactSize(inputs_.size()).Encode();
        output->Concatenate(inputCount.data(), inputCount.size());

        for (const auto& current : inputs_) { output += current.Encode(); }
        // ------------------------------------------------
        const auto outputCount =
            blockchain::bitcoin::CompactSize(outputs_.size()).Encode();
        output->Concatenate(outputCount.data(), outputCount.size());

        for (const auto& current : outputs_) { output += current.Encode(); }
        // ------------------------------------------------
        if (witness_flag_present_) {
            const auto witnessCount =
                blockchain::bitcoin::CompactSize(witnesses_.size()).Encode();
            output->Concatenate(witnessCount.data(), witnessCount.size());

            for (const auto& current : witnesses_) {
                output += current.Encode();
            }
        }
        // ------------------------------------------------
        TxLockTimeField raw_lock_time(lock_time_);
        output->Concatenate(&raw_lock_time, sizeof(raw_lock_time));

        return output;
    } catch (...) {
        return Data::Factory();
    }
}
}  // namespace opentxs::blockchain::transaction::bitcoin

namespace opentxs
{
namespace bitcoin = blockchain::transaction::bitcoin;

bitcoin::Transaction* bitcoin::Transaction::Factory(
    const api::internal::Core& api,
    const blockchain::Type network,
    const void* payload,
    const std::size_t size)
{
    using ReturnType = bitcoin::Transaction;

    auto* it{static_cast<const std::byte*>(payload)};
    std::size_t expectedSize{};
    bitcoin::TxDataFormatVersion data_format_version{};
    bool witness_flag_present{false};
    bitcoin::Inputs inputs;
    bitcoin::Outputs outputs;
    bitcoin::Witnesses witnesses;
    bitcoin::TxLockTime lock_time{};

    const bool decoded = bitcoin::Transaction::DecodeFromPayload(
        it,
        expectedSize,
        size,
        data_format_version,
        witness_flag_present,
        inputs,
        outputs,
        witnesses,
        lock_time);
    // -----------------------------------------------
    if (false == decoded) { return nullptr; }
    // -----------------------------------------------
    try {
        return new ReturnType(
            api,
            network,
            data_format_version,
            witness_flag_present,
            inputs,
            outputs,
            witnesses,
            lock_time);
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Checksum failure").Flush();

        return nullptr;
    }
}

bool bitcoin::Transaction::DecodeFromPayload(
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    bitcoin::TxDataFormatVersion& data_format_version,
    bool& witness_flag_present,
    bitcoin::Inputs& inputs,
    bitcoin::Outputs& outputs,
    bitcoin::Witnesses& witnesses,
    bitcoin::TxLockTime& lock_time)
{
    bitcoin::TxDataFormatVersionField versionField{};

    expectedSize += sizeof(versionField);

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Size below minimum.").Flush();

        return false;
    }
    // -----------------------------------------------
    OTPassword::safe_memcpy(
        &versionField, sizeof(versionField), it, sizeof(versionField));
    it += sizeof(versionField);

    data_format_version = versionField.value();
    // -----------------------------------------------
    // This optional field is not always present. Size is 0 or 2.
    //
    bitcoin::WitnessFlagField raw_witness_flag{};

    expectedSize += sizeof(raw_witness_flag);

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Size below minimum.").Flush();

        return false;
    }

    OTPassword::safe_memcpy(
        &raw_witness_flag,
        sizeof(raw_witness_flag),
        it,
        sizeof(raw_witness_flag));

    bitcoin::WitnessFlagArray witness_flag_array{raw_witness_flag[0].value(),
                                                 raw_witness_flag[1].value()};

    witness_flag_present = (witness_flag_array[1] & 1);

    if (witness_flag_present) {
        it += sizeof(raw_witness_flag);
    }  // Else witness flag is NOT present, and thus we don't increment.
    // -----------------------------------------------
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Size below minimum for TxInput count field")
            .Flush();

        return false;
    }

    std::size_t inputCount{0};
    const bool decodedInputCount =
        blockchain::bitcoin::DecodeCompactSizeFromPayload(
            it, expectedSize, size, inputCount);

    if (!decodedInputCount) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": CompactSize incomplete for input count field")
            .Flush();

        return false;
    }
    const std::size_t witnessCount = witness_flag_present ? inputCount : 0;
    // -----------------------------------------------
    // Read all the TxInputs.
    // Note: All the expectedSize checks are done inside
    // DecodeFromPayload. That's why I'm not checking that
    // here for each of these.
    //
    for (std::size_t ii = 0; ii < inputCount; ii++) {
        bitcoin::Input txInput;
        const bool decoded =
            Input::DecodeFromPayload(it, expectedSize, size, txInput);
        if (!decoded) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to parse a TxInput")
                .Flush();

            return false;
        }

        inputs.push_back(txInput);
    }
    // -----------------------------------------------
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Size below minimum for TxOutput count field")
            .Flush();

        return false;
    }

    std::size_t outputCount{0};
    const bool decodedOutputCount =
        blockchain::bitcoin::DecodeCompactSizeFromPayload(
            it, expectedSize, size, outputCount);

    if (!decodedOutputCount) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": CompactSize incomplete for output count field")
            .Flush();

        return false;
    }
    // -----------------------------------------------
    // Read all the TxOutputs.
    //
    for (std::size_t ii = 0; ii < outputCount; ii++) {
        bitcoin::Output txOutput;
        const bool decoded =
            Output::DecodeFromPayload(it, expectedSize, size, txOutput);
        if (!decoded) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to parse a TxOutput")
                .Flush();

            return false;
        }

        outputs.push_back(txOutput);
    }
    // -----------------------------------------------
    // Read the Witness data for each input
    //
    for (std::size_t ii = 0; ii < witnessCount; ii++) {
        bitcoin::Witness txWitness;
        const bool decoded =
            Witness::DecodeFromPayload(it, expectedSize, size, txWitness);
        if (!decoded) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to parse a Tx Witness field")
                .Flush();

            return false;
        }

        witnesses.push_back(txWitness);
    }
    // -----------------------------------------------
    bitcoin::TxLockTimeField raw_lock_time{};

    expectedSize += sizeof(raw_lock_time);

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Size below minimum for lock time field")
            .Flush();

        return false;
    }
    // -----------------------------------------------
    OTPassword::safe_memcpy(
        &raw_lock_time, sizeof(raw_lock_time), it, sizeof(raw_lock_time));
    it += sizeof(raw_lock_time);

    lock_time = raw_lock_time.value();

    return true;
}

bitcoin::Transaction* bitcoin::Transaction::Factory(
    const api::internal::Core& api,
    const blockchain::Type network,
    const bitcoin::TxDataFormatVersion data_format_version,
    const bool witness_flag_present,
    const bitcoin::Inputs& inputs,
    const bitcoin::Outputs& outputs,
    const bitcoin::Witnesses& witnesses,
    const bitcoin::TxLockTime lock_time)
{
    using ReturnType = bitcoin::Transaction;

    return new ReturnType(
        api,
        network,
        data_format_version,
        witness_flag_present,
        inputs,
        outputs,
        witnesses,
        lock_time);
}
}  // namespace opentxs
