// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/core/Log.hpp"

#include "blockchain/bitcoin/CompactSize.hpp"

#include <numeric>
#include <optional>
#include <vector>

#include "Outputs.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::block::bitcoin::implementation::Outputs::"

namespace opentxs
{
using ReturnType = blockchain::block::bitcoin::implementation::Outputs;

auto Factory::BitcoinTransactionOutputs(
    std::vector<std::unique_ptr<blockchain::block::bitcoin::Output>>&& outputs,
    std::optional<std::size_t> size) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Outputs>
{
    try {

        return std::make_unique<ReturnType>(std::move(outputs), size);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
Outputs::Outputs(
    std::vector<std::unique_ptr<bitcoin::Output>>&& outputs,
    std::optional<std::size_t> size) noexcept(false)
    : outputs_(std::move(outputs))
    , size_(size)
{
    for (const auto& output : outputs_) {
        if (false == bool(output)) {
            throw std::runtime_error("invalid output");
        }
    }
}

auto Outputs::CalculateSize() const noexcept -> std::size_t
{
    if (false == size_.has_value()) {
        const auto cs = blockchain::bitcoin::CompactSize(size());
        size_ = std::accumulate(
            cbegin(),
            cend(),
            cs.Size(),
            [](const std::size_t& lhs, const auto& rhs) -> std::size_t {
                return lhs + rhs.CalculateSize();
            });
    }

    return size_.value();
}

auto Outputs::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    if (!destination) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        return {};
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
    }

    const auto size = CalculateSize();
    auto output = destination(size);

    if (false == output.valid(size)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to allocate output bytes")
            .Flush();
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        return {};
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
    }

    auto remaining{output.size()};
    const auto cs = blockchain::bitcoin::CompactSize(this->size()).Encode();
    auto it = static_cast<std::byte*>(output.data());
    std::memcpy(static_cast<void*>(it), cs.data(), cs.size());
    std::advance(it, cs.size());
    remaining -= cs.size();

    for (const auto& row : outputs_) {
        OT_ASSERT(row);

        const auto bytes = row->Serialize(preallocated(remaining, it));

        if (false == bytes.has_value()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to serialize script")
                .Flush();
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
            return {};
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
        }

        std::advance(it, bytes.value());
        remaining -= bytes.value();
    }

    return size;
}

auto Outputs::Serialize(proto::BlockchainTransaction& destination) const
    noexcept -> bool
{
    auto index = std::uint32_t{0};

    for (const auto& output : outputs_) {
        OT_ASSERT(output);

        auto& out = *destination.add_output();

        if (false == output->Serialize(index, out)) { return false; }

        ++index;
    }

    return true;
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
