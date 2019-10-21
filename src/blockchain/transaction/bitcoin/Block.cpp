// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include <boost/endian/buffers.hpp>

#include <memory>
#include <vector>

#include "Transaction.hpp"

#include "Block.hpp"

#define OT_METHOD "opentxs::blockchain::transaction::bitcoin::Block::"

namespace be = boost::endian;

namespace opentxs::blockchain::transaction::bitcoin
{
using VersionField = be::little_int32_buf_t;
using HashField = std::array<std::byte, 32>;
using TimestampField32 = be::little_uint32_buf_t;
using DifficultyTargetField = be::little_uint32_buf_t;
using NonceField = be::little_uint32_buf_t;

Block::Block(
    const api::internal::Core& api,
    const blockchain::Type network,
    const block::Version block_version,
    const Data& previous_block_hash,
    const Data& merkle_root_hash,
    const Time timestamp,
    const std::uint32_t difficulty_target,
    const std::uint32_t nonce,
    const std::vector<std::shared_ptr<Transaction>>& transactions) noexcept
    : api_(api)
    , network_(network)
    , block_version_(block_version)
    , previous_block_hash_(previous_block_hash)
    , merkle_root_hash_(merkle_root_hash)
    , timestamp_(timestamp)
    , difficulty_target_(difficulty_target)
    , nonce_(nonce)
    , transactions_(transactions)
{
}

OTData Block::Encode() const noexcept
{
    try {
        VersionField versionField(static_cast<std::int32_t>(block_version_));
        auto output = Data::Factory(&versionField, sizeof(versionField));
        HashField raw_previous_block_hash{};
        OTPassword::safe_memcpy(
            &raw_previous_block_hash,
            sizeof(raw_previous_block_hash),
            previous_block_hash_->data(),
            previous_block_hash_->size());
        output->Concatenate(
            &raw_previous_block_hash, sizeof(raw_previous_block_hash));
        HashField raw_merkle_root_hash{};
        OTPassword::safe_memcpy(
            &raw_merkle_root_hash,
            sizeof(raw_merkle_root_hash),
            merkle_root_hash_->data(),
            merkle_root_hash_->size());
        output->Concatenate(
            &raw_merkle_root_hash, sizeof(raw_merkle_root_hash));
        std::time_t temp_time = Clock::to_time_t(timestamp_);
        TimestampField32 raw_timestamp(temp_time);
        output->Concatenate(&raw_timestamp, sizeof(raw_timestamp));
        DifficultyTargetField raw_difficulty_target(
            static_cast<std::uint32_t>(difficulty_target_));
        output->Concatenate(
            &raw_difficulty_target, sizeof(raw_difficulty_target));
        NonceField raw_nonce(static_cast<std::uint32_t>(nonce_));
        output->Concatenate(&raw_nonce, sizeof(raw_nonce));
        const auto txCount =
            blockchain::bitcoin::CompactSize(transactions_.size()).Encode();
        output->Concatenate(txCount.data(), txCount.size());

        for (const auto& current : transactions_) {
            output += current->Encode();
        }

        return output;
    } catch (...) {
        return Data::Factory();
    }
}
}  // namespace opentxs::blockchain::transaction::bitcoin

namespace opentxs
{
namespace bitcoin = blockchain::transaction::bitcoin;

bitcoin::Block* bitcoin::Block::Factory(
    const api::internal::Core& api,
    const blockchain::Type network,
    const void* payload,
    const std::size_t size)
{
    using ReturnType = bitcoin::Block;
    auto* it{static_cast<const std::byte*>(payload)};
    std::size_t expectedSize{};
    block::Version block_version{};
    OTData previous_block_hash = Data::Factory();
    OTData merkle_root_hash = Data::Factory();
    Time timestamp{};
    std::uint32_t difficulty_target{};
    std::uint32_t nonce{};
    std::vector<std::shared_ptr<Transaction>> transactions;
    const bool decoded = bitcoin::Block::DecodeFromPayload(
        api,
        network,
        it,
        expectedSize,
        size,
        block_version,
        previous_block_hash,
        merkle_root_hash,
        timestamp,
        difficulty_target,
        nonce,
        transactions);

    if (false == decoded) { return nullptr; }

    try {
        return new ReturnType(
            api,
            network,
            block_version,
            previous_block_hash,
            merkle_root_hash,
            timestamp,
            difficulty_target,
            nonce,
            transactions);
    } catch (...) {
        opentxs::LogOutput(OT_METHOD)(__FUNCTION__)(": Checksum failure")
            .Flush();

        return nullptr;
    }
}

bool bitcoin::Block::DecodeFromPayload(
    const opentxs::api::internal::Core& api,
    const opentxs::blockchain::Type network,
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    block::Version& block_version,
    Data& previous_block_hash,
    Data& merkle_root_hash,
    Time& timestamp,
    std::uint32_t& difficulty_target,
    std::uint32_t& nonce,
    std::vector<std::shared_ptr<Transaction>>& transactions) noexcept
{
    bitcoin::VersionField versionField{};
    expectedSize += sizeof(versionField);

    if (expectedSize > size) {
        opentxs::LogOutput(OT_METHOD)(__FUNCTION__)(": Size below minimum.")
            .Flush();

        return false;
    }

    OTPassword::safe_memcpy(
        &versionField, sizeof(versionField), it, sizeof(versionField));
    it += sizeof(versionField);
    block_version = versionField.value();
    HashField raw_previous_block_hash{};
    expectedSize += sizeof(raw_previous_block_hash);

    if (expectedSize > size) {
        opentxs::LogOutput(OT_METHOD)(__FUNCTION__)(": Size below minimum.")
            .Flush();

        return false;
    }

    OTPassword::safe_memcpy(
        &raw_previous_block_hash,
        sizeof(raw_previous_block_hash),
        it,
        sizeof(raw_previous_block_hash));
    it += sizeof(raw_previous_block_hash);
    previous_block_hash.Assign(
        &raw_previous_block_hash, sizeof(raw_previous_block_hash));
    HashField raw_merkle_root_hash{};
    expectedSize += sizeof(raw_merkle_root_hash);

    if (expectedSize > size) {
        opentxs::LogOutput(OT_METHOD)(__FUNCTION__)(": Size below minimum.")
            .Flush();

        return false;
    }

    OTPassword::safe_memcpy(
        &raw_merkle_root_hash,
        sizeof(raw_merkle_root_hash),
        it,
        sizeof(raw_merkle_root_hash));
    it += sizeof(raw_merkle_root_hash);
    merkle_root_hash.Assign(&merkle_root_hash, sizeof(merkle_root_hash));
    TimestampField32 raw_timestamp{};
    expectedSize += sizeof(raw_timestamp);

    if (expectedSize > size) {
        opentxs::LogOutput(OT_METHOD)(__FUNCTION__)(": Size below minimum.")
            .Flush();

        return false;
    }

    OTPassword::safe_memcpy(
        &raw_timestamp, sizeof(raw_timestamp), it, sizeof(raw_timestamp));
    it += sizeof(raw_timestamp);
    timestamp = Clock::from_time_t(raw_timestamp.value());
    DifficultyTargetField raw_difficulty_target{};
    expectedSize += sizeof(raw_difficulty_target);

    if (expectedSize > size) {
        opentxs::LogOutput(OT_METHOD)(__FUNCTION__)(": Size below minimum.")
            .Flush();

        return false;
    }

    OTPassword::safe_memcpy(
        &raw_difficulty_target,
        sizeof(raw_difficulty_target),
        it,
        sizeof(raw_difficulty_target));
    it += sizeof(raw_difficulty_target);
    difficulty_target = raw_difficulty_target.value();
    NonceField raw_nonce{};
    expectedSize += sizeof(raw_nonce);

    if (expectedSize > size) {
        opentxs::LogOutput(OT_METHOD)(__FUNCTION__)(": Size below minimum.")
            .Flush();

        return false;
    }

    OTPassword::safe_memcpy(
        &raw_nonce, sizeof(raw_nonce), it, sizeof(raw_nonce));
    it += sizeof(raw_nonce);
    nonce = raw_nonce.value();
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        opentxs::LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Size below minimum for Tx count field")
            .Flush();

        return false;
    }

    auto txCount = std::size_t{0};
    const bool decodedTxCount =
        blockchain::bitcoin::DecodeCompactSizeFromPayload(
            it, expectedSize, size, txCount);

    if (!decodedTxCount) {
        opentxs::LogOutput(OT_METHOD)(__FUNCTION__)(
            ": CompactSize incomplete for Tx count field")
            .Flush();

        return false;
    }

    // -----------------------------------------------
    // Read all the transactions.
    // Note: All the expectedSize checks are done inside
    // DecodeFromPayload. That's why I'm not checking that
    // here for each of these.
    //
    for (std::size_t ii = 0; ii < txCount; ii++) {
        bitcoin::TxDataFormatVersion data_format_version{};
        bool witness_flag_present{false};
        bitcoin::Inputs inputs;
        bitcoin::Outputs outputs;
        bitcoin::Witnesses witnesses;
        bitcoin::TxLockTime lock_time{};

        const bool decoded = Transaction::DecodeFromPayload(
            it,
            expectedSize,
            size,
            data_format_version,
            witness_flag_present,
            inputs,
            outputs,
            witnesses,
            lock_time);
        if (!decoded) {
            opentxs::LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to parse a Tx")
                .Flush();

            return false;
        }

        std::shared_ptr<bitcoin::Transaction> pTx(bitcoin::Transaction::Factory(
            api,
            network,
            data_format_version,
            witness_flag_present,
            inputs,
            outputs,
            witnesses,
            lock_time));

        transactions.push_back(pTx);
    }

    return true;
}

bitcoin::Block* bitcoin::Block::Factory(
    const opentxs::api::internal::Core& api,
    const opentxs::blockchain::Type network,
    const block::Version block_version,
    const Data& previous_block_hash,
    const Data& merkle_root_hash,
    const Time timestamp,
    const std::uint32_t difficulty_target,
    const std::uint32_t nonce,
    const std::vector<std::shared_ptr<Transaction>>& transactions)
{
    using ReturnType = bitcoin::Block;

    return new ReturnType(
        api,
        network,
        block_version,
        previous_block_hash,
        merkle_root_hash,
        timestamp,
        difficulty_target,
        nonce,
        transactions);
}
}  // namespace opentxs
