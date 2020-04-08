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
#include "internal/blockchain/block/Block.hpp"

#include <boost/endian/buffers.hpp>

#include <algorithm>
#include <cstring>
#include <limits>
#include <optional>

#include "Input.hpp"

namespace be = boost::endian;

#define OT_METHOD "opentxs::blockchain::block::bitcoin::implementation::Input::"

namespace opentxs
{
using ReturnType = blockchain::block::bitcoin::implementation::Input;

auto Factory::BitcoinTransactionInput(
    const api::Core& api,
    const ReadView outpoint,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    const ReadView sequence,
    const bool coinbase,
    std::vector<Space>&& witness) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Input>
{
    try {
        auto buf = be::little_uint32_buf_t{};

        if (sequence.size() != sizeof(buf)) {
            throw std::runtime_error("Invalid sequence");
        }

        std::memcpy(static_cast<void*>(&buf), sequence.data(), sequence.size());

        if (coinbase) {
            return std::make_unique<ReturnType>(
                api,
                buf.value(),
                blockchain::block::bitcoin::Outpoint{outpoint},
                std::move(witness),
                script,
                ReturnType::default_version_,
                outpoint.size() + cs.Total() + sequence.size());
        } else {
            return std::make_unique<ReturnType>(
                api,
                buf.value(),
                blockchain::block::bitcoin::Outpoint{outpoint},
                std::move(witness),
                opentxs::Factory::BitcoinScript(script, false, false),
                ReturnType::default_version_,
                outpoint.size() + cs.Total() + sequence.size());
        }
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::BitcoinTransactionInput(
    const api::Core& api,
    const proto::BlockchainTransactionInput in,
    const bool coinbase) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::Input>
{
    const auto& outpoint = in.previous();
    auto witness = std::vector<Space>{};

    for (const auto& bytes : in.witness().item()) {
        const auto it = reinterpret_cast<const std::byte*>(bytes.data());
        witness.emplace_back(it, it + bytes.size());
    }

    try {
        if (coinbase) {
            return std::make_unique<ReturnType>(
                api,
                in.sequence(),
                blockchain::block::bitcoin::Outpoint{
                    outpoint.txid(),
                    static_cast<std::uint32_t>(outpoint.index())},
                std::move(witness),
                in.script(),
                in.version());
        } else {
            return std::make_unique<ReturnType>(
                api,
                in.sequence(),
                blockchain::block::bitcoin::Outpoint{
                    outpoint.txid(),
                    static_cast<std::uint32_t>(outpoint.index())},
                std::move(witness),
                opentxs::Factory::BitcoinScript(in.script(), false, coinbase),
                in.version());
        }
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

auto Outpoint::Bytes() const noexcept -> ReadView
{
    return ReadView{reinterpret_cast<const char*>(this), sizeof(*this)};
}

auto Outpoint::operator<(const Outpoint& rhs) const noexcept -> bool
{
    return 0 > std::memcmp(this, &rhs, sizeof(*this));
}

auto Outpoint::operator<=(const Outpoint& rhs) const noexcept -> bool
{
    return 0 >= std::memcmp(this, &rhs, sizeof(*this));
}

auto Outpoint::operator>(const Outpoint& rhs) const noexcept -> bool
{
    return 0 < std::memcmp(this, &rhs, sizeof(*this));
}

auto Outpoint::operator>=(const Outpoint& rhs) const noexcept -> bool
{
    return 0 <= std::memcmp(this, &rhs, sizeof(*this));
}

auto Outpoint::operator==(const Outpoint& rhs) const noexcept -> bool
{
    return 0 == std::memcmp(this, &rhs, sizeof(*this));
}

auto Outpoint::operator!=(const Outpoint& rhs) const noexcept -> bool
{
    return 0 != std::memcmp(this, &rhs, sizeof(*this));
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
    const api::Core& api,
    const std::uint32_t sequence,
    Outpoint&& previous,
    std::vector<Space>&& witness,
    std::unique_ptr<const bitcoin::Script> script,
    Space&& coinbase,
    const VersionNumber version,
    std::optional<std::size_t> size) noexcept(false)
    : api_(api)
    , serialize_version_(version)
    , previous_(std::move(previous))
    , witness_(std::move(witness))
    , script_(std::move(script))
    , coinbase_(std::move(coinbase))
    , sequence_(sequence)
    , size_(size)
    , normalized_size_()
{
    if (false == bool(script_)) {
        throw std::runtime_error("Invalid input script");
    }
}

Input::Input(
    const api::Core& api,
    const std::uint32_t sequence,
    Outpoint&& previous,
    std::vector<Space>&& witness,
    std::unique_ptr<const bitcoin::Script> script,
    const VersionNumber version,
    std::optional<std::size_t> size) noexcept(false)
    : Input(
          api,
          sequence,
          std::move(previous),
          std::move(witness),
          std::move(script),
          Space{},
          version,
          size)
{
}

Input::Input(
    const api::Core& api,
    const std::uint32_t sequence,
    Outpoint&& previous,
    std::vector<Space>&& witness,
    const ReadView coinbase,
    const VersionNumber version,
    std::optional<std::size_t> size) noexcept(false)
    : Input(
          api,
          sequence,
          std::move(previous),
          std::move(witness),
          Factory::BitcoinScript(ScriptElements{}, false, true),
          Space{reinterpret_cast<const std::byte*>(coinbase.data()),
                reinterpret_cast<const std::byte*>(coinbase.data()) +
                    coinbase.size()},
          version,
          size)
{
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

auto Input::ExtractElements(const filter::Type style) const noexcept
    -> std::vector<Space>
{
    auto output = std::vector<Space>{};

    switch (style) {
        case filter::Type::Extended_opentxs: {
            LogTrace(OT_METHOD)(__FUNCTION__)(": processing input script")
                .Flush();
            output = script_->ExtractElements(style);

            for (const auto& data : witness_) {
                auto it{data.data()};

                switch (data.size()) {
                    case 65: {
                        std::advance(it, 1);
                        [[fallthrough]];
                    }
                    case 64: {
                        output.emplace_back(it, it + 32);
                        std::advance(it, 32);
                        output.emplace_back(it, it + 32);
                        [[fallthrough]];
                    }
                    case 33:
                    case 32:
                    case 20: {
                        output.emplace_back(data.cbegin(), data.cend());
                    } break;
                    default: {
                    }
                }
            }

            [[fallthrough]];
        }
        case filter::Type::Basic_BCHVariant: {
            LogTrace(OT_METHOD)(__FUNCTION__)(": processing consumed outpoint")
                .Flush();
            auto it = reinterpret_cast<const std::byte*>(&previous_);
            output.emplace_back(it, it + sizeof(previous_));
        } break;
        case filter::Type::Basic_BIP158:
        default: {
            LogTrace(OT_METHOD)(__FUNCTION__)(": skipping input").Flush();
        }
    }

    LogTrace(OT_METHOD)(__FUNCTION__)(": extracted ")(output.size())(
        " elements")
        .Flush();

    return output;
}

auto Input::FindMatches(
    const ReadView txid,
    const FilterType type,
    const Patterns& txos,
    const Patterns& patterns) const noexcept -> Matches
{
    auto output = SetIntersection(api_, txid, patterns, ExtractElements(type));

    if (0 < txos.size()) {
        auto outpoint = std::vector<Space>{};
        auto it = reinterpret_cast<const std::byte*>(&previous_);
        outpoint.emplace_back(it, it + sizeof(previous_));
        auto temp = SetIntersection(api_, txid, txos, outpoint);
        output.insert(
            output.end(),
            std::make_move_iterator(temp.begin()),
            std::make_move_iterator(temp.end()));
    }

    return output;
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
