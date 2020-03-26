// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/Bytes.hpp"

#include "blockchain/bitcoin/CompactSize.hpp"

#include "Output.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::block::bitcoin::implementation::Output::"

namespace opentxs
{
using ReturnType = blockchain::block::bitcoin::implementation::Output;

auto Factory::BitcoinTransactionOutput(
    const std::int64_t value,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Output>
{
    return std::make_unique<ReturnType>(
        value, sizeof(value) + cs.Total(), script);
}

auto Factory::BitcoinTransactionOutput(
    const proto::BlockchainTransactionOutput in) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Output>
{
    try {
        auto value = static_cast<std::int64_t>(in.value());
        auto cs = blockchain::bitcoin::CompactSize(in.script().size());

        return std::make_unique<ReturnType>(
            value, sizeof(value) + cs.Total(), in.script(), in.version());
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
    const VersionNumber version,
    const std::int64_t value,
    std::unique_ptr<const bitcoin::Script> script,
    std::optional<std::size_t> size) noexcept(false)
    : serialize_version_(version)
    , value_(value)
    , script_(std::move(script))
    , size_(size)
{
    if (false == bool(script_)) {
        throw std::runtime_error("Invalid output script");
    }

    if (0 > value_) { throw std::runtime_error("Invalid output value"); }
}

Output::Output(const std::int64_t value, ScriptElements&& in) noexcept(false)
    : Output(
          default_version_,
          value,
          opentxs::Factory::BitcoinScript(std::move(in), true, false))
{
}

Output::Output(
    const std::int64_t value,
    const std::size_t size,
    const ReadView in,
    const VersionNumber version) noexcept(false)
    : Output(
          version,
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

auto Output::Serialize(
    const std::uint32_t index,
    proto::BlockchainTransactionOutput& out) const noexcept -> bool
{
    OT_ASSERT(0 <= value_);

    out.set_version(std::max(default_version_, serialize_version_));
    out.set_index(index);
    out.set_value(value_);

    if (false == script_->Serialize(writer(*out.mutable_script()))) {
        return false;
    }

    /* TODO
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
