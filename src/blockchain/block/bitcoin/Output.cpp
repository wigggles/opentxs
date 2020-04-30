// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/block/bitcoin/Output.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>

#include "Factory.hpp"
#include "blockchain/bitcoin/CompactSize.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::block::bitcoin::implementation::Output::"

namespace opentxs
{
using ReturnType = blockchain::block::bitcoin::implementation::Output;

auto Factory::BitcoinTransactionOutput(
    const api::Core& api,
    const std::uint32_t index,
    const std::int64_t value,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Output>
{
    try {
        return std::make_unique<ReturnType>(
            api, index, value, sizeof(value) + cs.Total(), script);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::BitcoinTransactionOutput(
    const api::Core& api,
    const proto::BlockchainTransactionOutput in) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Output>
{
    try {
        auto value = static_cast<std::int64_t>(in.value());
        auto cs = blockchain::bitcoin::CompactSize(in.script().size());

        return std::make_unique<ReturnType>(
            api,
            in.index(),
            value,
            sizeof(value) + cs.Total(),
            in.script(),
            in.version());
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
const VersionNumber Output::default_version_{1};

Output::Output(
    const api::Core& api,
    const VersionNumber version,
    const std::uint32_t index,
    const std::int64_t value,
    std::unique_ptr<const bitcoin::Script> script,
    std::optional<std::size_t> size) noexcept(false)
    : api_(api)
    , serialize_version_(version)
    , index_(index)
    , value_(value)
    , script_(std::move(script))
    , size_(size)
    , payee_(api_.Factory().Identifier())
    , payer_(api_.Factory().Identifier())
    , keys_()
{
    if (false == bool(script_)) {
        throw std::runtime_error("Invalid output script");
    }

    if (0 > value_) { throw std::runtime_error("Invalid output value"); }
}

Output::Output(
    const api::Core& api,
    const std::uint32_t index,
    const std::int64_t value,
    const std::size_t size,
    const ReadView in,
    const VersionNumber version) noexcept(false)
    : Output(
          api,
          version,
          index,
          value,
          opentxs::Factory::BitcoinScript(in, true, false),
          size)
{
}

auto Output::CalculateSize() const noexcept -> std::size_t
{
    if (false == size_.has_value()) {
        const auto scriptCS =
            blockchain::bitcoin::CompactSize(script_->CalculateSize());
        size_ = sizeof(value_) + scriptCS.Total();
    }

    return size_.value();
}

auto Output::ExtractElements(const filter::Type style) const noexcept
    -> std::vector<Space>
{
    return script_->ExtractElements(style);
}

auto Output::FindMatches(
    const ReadView txid,
    const FilterType type,
    const Patterns& patterns) const noexcept -> Matches
{
    // TODO set payee_, payer_, and keys_ based on match results

    return SetIntersection(api_, txid, patterns, ExtractElements(type));
}

auto Output::MergeMetadata(const SerializeType& rhs) const noexcept -> void
{
    // TODO proto::BlockchainTransactionOutput does not yet have correct fields
}

auto Output::Serialize(const AllocateOutput destination) const noexcept
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

    const auto scriptCS =
        blockchain::bitcoin::CompactSize(script_->CalculateSize());
    const auto csData = scriptCS.Encode();
    auto it = static_cast<std::byte*>(output.data());
    std::memcpy(static_cast<void*>(it), &value_, sizeof(value_));
    std::advance(it, sizeof(value_));
    std::memcpy(static_cast<void*>(it), csData.data(), csData.size());
    std::advance(it, csData.size());

    if (script_->Serialize(preallocated(scriptCS.Value(), it))) {

        return size;
    } else {
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
}

auto Output::Serialize(SerializeType& out) const noexcept -> bool
{
    OT_ASSERT(0 <= value_);

    out.set_version(std::max(default_version_, serialize_version_));
    out.set_index(index_);
    out.set_value(value_);

    if (false == script_->Serialize(writer(*out.mutable_script()))) {
        return false;
    }

    /* TODO these fields must be changed
    oneof destination
    {
        BlockchainWalletKey key = 5;
        BlockchainExternalAddress external = 6;
    }

    optional string confirmedspend = 7;
    repeated string orphanedspend = 8;
    */

    return true;
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
