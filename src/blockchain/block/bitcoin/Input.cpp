// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/core/Log.hpp"

#include "blockchain/bitcoin/CompactSize.hpp"

#include <boost/endian/buffers.hpp>

#include <limits>
#include <optional>

#include "Input.hpp"

namespace be = boost::endian;

#define OT_METHOD "opentxs::blockchain::block::bitcoin::implementation::Input::"

namespace opentxs
{
using ReturnType = blockchain::block::bitcoin::implementation::Input;

auto Factory::BitcoinTransactionInput(
    const ReadView outpoint,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    const ReadView sequence,
    const bool coinbase) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Input>
{
    try {
        auto buf = be::little_uint32_buf_t{};

        if (sequence.size() != sizeof(buf)) {
            throw std::runtime_error("Invalid sequence");
        }

        std::memcpy(static_cast<void*>(&buf), sequence.data(), sequence.size());

        return std::make_unique<ReturnType>(
            buf.value(),
            blockchain::block::bitcoin::Outpoint{outpoint},
            opentxs::Factory::BitcoinScript(script, false, coinbase),
            ReturnType::default_version_,
            outpoint.size() + cs.Total() + sequence.size());
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::BitcoinTransactionInput(
    const proto::BlockchainTransactionInput in,
    const bool coinbase) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Input>
{
    const auto& outpoint = in.previous();

    try {
        return std::make_unique<ReturnType>(
            in.sequence(),
            blockchain::block::bitcoin::Outpoint{
                outpoint.txid(), static_cast<std::uint32_t>(outpoint.index())},
            opentxs::Factory::BitcoinScript(in.script(), false, coinbase),
            in.version());
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin
{
Outpoint::Outpoint(const ReadView in) noexcept(false)
    : txid_()
    , index_()
{
    if (in.size() < sizeof(*this)) {
        throw std::runtime_error("Invalid bytes");
    }

    std::memcpy(static_cast<void*>(this), in.data(), sizeof(*this));
}

Outpoint::Outpoint(const ReadView txid, const std::uint32_t index) noexcept(
    false)
    : txid_()
    , index_()
{
    if (txid_.size() != txid.size()) {
        throw std::runtime_error("Invalid txid");
    }

    const auto buf = be::little_uint32_buf_t{index};

    static_assert(sizeof(index_) == sizeof(buf));

    std::memcpy(static_cast<void*>(txid_.data()), txid.data(), txid_.size());
    std::memcpy(static_cast<void*>(index_.data()), &buf, index_.size());
}

auto Outpoint::Index() const noexcept -> std::uint32_t
{
    auto buf = be::little_uint32_buf_t{};

    static_assert(sizeof(index_) == sizeof(buf));

    std::memcpy(static_cast<void*>(&buf), index_.data(), index_.size());

    return buf.value();
}

auto Outpoint::Txid() const noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(txid_.data()), txid_.size()};
}
}  // namespace opentxs::blockchain::block::bitcoin

namespace opentxs::blockchain::block::bitcoin::implementation
{
const VersionNumber Input::default_version_{1};
const VersionNumber Input::outpoint_version_{1};

Input::Input(
    const std::uint32_t sequence,
    Outpoint&& previous,
    std::unique_ptr<const bitcoin::Script> script,
    const VersionNumber version,
    std::optional<std::size_t> size) noexcept(false)
    : serialize_version_(version)
    , previous_(std::move(previous))
    , script_(std::move(script))
    , sequence_(sequence)
    , size_(size)
    , normalized_size_()
{
    if (false == bool(script_)) {
        throw std::runtime_error("Invalid input script");
    }
}

auto Input::CalculateSize(const bool normalized) const noexcept -> std::size_t
{
    auto& output = normalized ? normalized_size_ : size_;

    if (false == output.has_value()) {
        const auto cs =
            blockchain::bitcoin::CompactSize(script_->CalculateSize());
        output = sizeof(previous_) + (normalized ? 1 : cs.Total()) +
                 sizeof(sequence_);
    }

    return output.value();
}

auto Input::serialize(const AllocateOutput destination, const bool normalized)
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

    const auto size = CalculateSize(normalized);
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

    auto it = static_cast<std::byte*>(output.data());
    std::memcpy(static_cast<void*>(it), &previous_, sizeof(previous_));
    std::advance(it, sizeof(previous_));
    const auto cs =
        normalized ? blockchain::bitcoin::CompactSize(0)
                   : blockchain::bitcoin::CompactSize(script_->CalculateSize());
    const auto csData = cs.Encode();
    std::memcpy(static_cast<void*>(it), csData.data(), csData.size());
    std::advance(it, csData.size());

    if (false == normalized) {
        if (false == script_->Serialize(preallocated(cs.Value(), it))) {
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

        std::advance(it, cs.Value());
    }

    auto buf = be::little_uint32_buf_t{sequence_};
    std::memcpy(static_cast<void*>(it), &buf, sizeof(buf));

    return size;
}

auto Input::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(destination, false);
}

auto Input::Serialize(
    const std::uint32_t index,
    proto::BlockchainTransactionInput& out) const noexcept -> bool
{
    out.set_version(std::max(default_version_, serialize_version_));
    out.set_index(index);

    if (false == script_->Serialize(writer(*out.mutable_script()))) {

        return false;
    }

    out.set_sequence(sequence_);
    auto& outpoint = *out.mutable_previous();
    outpoint.set_version(outpoint_version_);
    outpoint.set_txid(std::string{previous_.Txid()});
    outpoint.set_index(previous_.Index());

    return true;
}

auto Input::SerializeNormalized(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    return serialize(destination, true);
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
