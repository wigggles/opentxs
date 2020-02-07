// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "TransactionSection.hpp"
#include "Input.hpp"
#include "Output.hpp"
#include "Witness.hpp"

#include <boost/endian/buffers.hpp>

namespace opentxs::blockchain::transaction::bitcoin
{
namespace be = boost::endian;

using Inputs = std::vector<Input>;
using Outputs = std::vector<Output>;
using Witnesses = std::vector<Witness>;

using TxLockTime = std::uint32_t;
using TxLockTimeField = be::little_uint32_buf_t;

using TxDataFormatVersion = std::int32_t;
using TxDataFormatVersionField = be::little_int32_buf_t;

using WitnessFlagArray = std::array<std::uint8_t, 2>;
using WitnessFlagField = std::array<be::little_uint8_buf_t, 2>;

class Transaction
{
public:
    const Inputs& inputs() const noexcept { return inputs_; }
    const Outputs& outputs() const noexcept { return outputs_; }
    const Witnesses& witnesses() const noexcept { return witnesses_; }

    TxLockTime lockTime() const noexcept { return lock_time_; }

    bool hasWitnessData() const noexcept { return witness_flag_present_; }

    TxDataFormatVersion Version() const noexcept
    {
        return data_format_version_;
    }

    OTData Encode() const noexcept;

    static Transaction* Factory(
        const opentxs::api::internal::Core& api,
        const opentxs::blockchain::Type network,
        const void* payload,
        const std::size_t size);

    static Transaction* Factory(
        const opentxs::api::internal::Core& api,
        const opentxs::blockchain::Type network,
        const TxDataFormatVersion data_format_version,
        const bool witness_flag_present,
        const Inputs& inputs,
        const Outputs& outputs,
        const Witnesses& witnesses,
        const TxLockTime lock_time);

    static bool DecodeFromPayload(
        const std::byte*& it,
        std::size_t& expectedSize,
        const std::size_t size,
        TxDataFormatVersion& data_format_version,
        bool& witness_flag_present,
        Inputs& inputs,
        Outputs& outputs,
        Witnesses& witnesses,
        TxLockTime& lock_time);

    Transaction() = delete;
    Transaction(const Transaction&) = delete;
    Transaction(Transaction&&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    Transaction& operator=(Transaction&&) = delete;

protected:
    const api::internal::Core& api_;
    const blockchain::Type network_;

private:
    const TxDataFormatVersion data_format_version_;
    const bool witness_flag_present_;
    const Inputs inputs_;
    const Outputs outputs_;
    const Witnesses witnesses_;
    const TxLockTime lock_time_;

    Transaction(
        const api::internal::Core& api,
        const blockchain::Type network,
        const TxDataFormatVersion data_format_version,
        const bool witness_flag_present,
        const Inputs& inputs,
        const Outputs& outputs,
        const Witnesses& witnesses,
        const TxLockTime lock_time) noexcept;
};
}  // namespace opentxs::blockchain::transaction::bitcoin
