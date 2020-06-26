// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/block/bitcoin/Inputs.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/BlockchainTransaction.pb.h"
#include "util/Container.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::block::bitcoin::implementation::Inputs::"

namespace opentxs::factory
{
using ReturnType = blockchain::block::bitcoin::implementation::Inputs;

auto BitcoinTransactionInputs(
    std::vector<std::unique_ptr<blockchain::block::bitcoin::internal::Input>>&&
        inputs,
    std::optional<std::size_t> size) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Inputs>
{
    try {

        return std::make_unique<ReturnType>(std::move(inputs), size);
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin::implementation
{
Inputs::Inputs(InputList&& inputs, std::optional<std::size_t> size) noexcept(
    false)
    : inputs_(std::move(inputs))
    , size_(size)
    , normalized_size_()
{
    for (const auto& input : inputs_) {
        if (false == bool(input)) { throw std::runtime_error("invalid input"); }
    }
}

Inputs::Inputs(const Inputs& rhs) noexcept
    : inputs_(clone(rhs.inputs_))
    , size_(rhs.size_)
    , normalized_size_(rhs.normalized_size_)
{
}

auto Inputs::AssociatedLocalNyms(std::vector<OTNymID>& output) const noexcept
    -> void
{
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& item) {
            item->AssociatedLocalNyms(output);
        });
}

auto Inputs::AssociatedRemoteContacts(
    std::vector<OTIdentifier>& output) const noexcept -> void
{
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& item) {
            item->AssociatedRemoteContacts(output);
        });
}

auto Inputs::CalculateSize(const bool normalized) const noexcept -> std::size_t
{
    auto& output = normalized ? normalized_size_ : size_;

    if (false == output.has_value()) {
        const auto cs = blockchain::bitcoin::CompactSize(size());
        output = std::accumulate(
            cbegin(),
            cend(),
            cs.Size(),
            [=](const std::size_t& lhs, const auto& rhs) -> std::size_t {
                return lhs + rhs.CalculateSize(normalized);
            });
    }

    return output.value();
}

auto Inputs::clone(const InputList& rhs) noexcept -> InputList
{
    auto output = InputList{};
    std::transform(
        std::begin(rhs),
        std::end(rhs),
        std::back_inserter(output),
        [](const auto& in) { return in->clone(); });

    return output;
}

auto Inputs::ExtractElements(const filter::Type style) const noexcept
    -> std::vector<Space>
{
    auto output = std::vector<Space>{};
    LogTrace(OT_METHOD)(__FUNCTION__)(": processing ")(size())(" inputs")
        .Flush();

    for (const auto& txin : *this) {
        auto temp = txin.ExtractElements(style);
        output.insert(
            output.end(),
            std::make_move_iterator(temp.begin()),
            std::make_move_iterator(temp.end()));
    }

    LogTrace(OT_METHOD)(__FUNCTION__)(": extracted ")(output.size())(
        " elements")
        .Flush();

    return output;
}

auto Inputs::FindMatches(
    const ReadView txid,
    const FilterType type,
    const Patterns& txos,
    const Patterns& patterns) const noexcept -> Matches
{
    auto output = Matches{};

    for (const auto& txin : *this) {
        auto temp = txin.FindMatches(txid, type, txos, patterns);
        output.insert(
            output.end(),
            std::make_move_iterator(temp.begin()),
            std::make_move_iterator(temp.end()));
    }

    return output;
}

auto Inputs::GetPatterns() const noexcept -> std::vector<PatternID>
{
    auto output = std::vector<PatternID>{};
    std::for_each(
        std::begin(inputs_), std::end(inputs_), [&](const auto& txin) {
            const auto patterns = txin->GetPatterns();
            output.insert(output.end(), patterns.begin(), patterns.end());
        });

    dedup(output);

    return output;
}

auto Inputs::NetBalanceChange(const identifier::Nym& nym) const noexcept
    -> opentxs::Amount
{
    return std::accumulate(
        std::begin(inputs_),
        std::end(inputs_),
        opentxs::Amount{0},
        [&](const auto prev, const auto& input) -> auto {
            return prev + input->NetBalanceChange(nym);
        });
}

auto Inputs::serialize(const AllocateOutput destination, const bool normalize)
    const noexcept -> std::optional<std::size_t>
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

    const auto size = CalculateSize(normalize);
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

    for (const auto& row : inputs_) {
        OT_ASSERT(row);

        const auto bytes =
            normalize ? row->SerializeNormalized(preallocated(remaining, it))
                      : row->Serialize(preallocated(remaining, it));

        if (false == bytes.has_value()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to serialize input")
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

auto Inputs::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(destination, false);
}

auto Inputs::Serialize(proto::BlockchainTransaction& destination) const noexcept
    -> bool
{
    auto index = std::uint32_t{0};

    for (const auto& input : inputs_) {
        OT_ASSERT(input);

        auto& out = *destination.add_input();

        if (false == input->Serialize(index, out)) { return false; }

        ++index;
    }

    return true;
}

auto Inputs::SerializeNormalized(const AllocateOutput destination)
    const noexcept -> std::optional<std::size_t>
{
    return serialize(destination, true);
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
