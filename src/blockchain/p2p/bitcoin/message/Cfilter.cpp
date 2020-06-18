// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Cfilter.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

// #define OT_METHOD "opentxs::blockchain::p2p::bitcoin::message::Cfilter::"

namespace opentxs::factory
{
auto BitcoinP2PCfilter(
    const api::client::Manager& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Cfilter*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Cfilter;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(": Invalid header")
            .Flush();

        return nullptr;
    }

    const auto& header = *pHeader;
    auto raw = ReturnType::BitcoinFormat{};
    auto expectedSize = sizeof(raw);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(
            ": Payload too short (begin)")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    std::memcpy(reinterpret_cast<std::byte*>(&raw), it, sizeof(raw));
    std::advance(it, sizeof(raw));
    expectedSize += 1;

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(
            ": Payload too short (compactsize)")
            .Flush();

        return nullptr;
    }

    auto filterSize = std::size_t{0};
    const auto haveSize = blockchain::bitcoin::DecodeCompactSizeFromPayload(
        it, expectedSize, size, filterSize);

    if (false == haveSize) {
        LogOutput(__FUNCTION__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    if ((expectedSize + filterSize) > size) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(
            ": Payload too short (filter)")
            .Flush();

        return nullptr;
    }

    expectedSize += 1;
    auto elementCount = std::size_t{0};
    auto csBytes = std::size_t{0};
    const auto haveElementCount =
        blockchain::bitcoin::DecodeCompactSizeFromPayload(
            it, expectedSize, size, elementCount, csBytes);

    if (false == haveElementCount) {
        LogOutput(__FUNCTION__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
    // std::size_t might be 32 bit
    if (std::numeric_limits<std::uint32_t>::max() < elementCount) {
        LogOutput(__FUNCTION__)(": Too many elements").Flush();

        return nullptr;
    }
#pragma GCC diagnostic pop

    const auto filterType = raw.Type(header.Network());
    const auto dataSize = filterSize - (1 + csBytes);

    return new ReturnType(
        api,
        std::move(pHeader),
        filterType,
        raw.Hash(),
        static_cast<std::uint32_t>(elementCount),
        Space{it, it + dataSize});
}

auto BitcoinP2PCfilter(
    const api::client::Manager& api,
    const blockchain::Type network,
    const blockchain::filter::Type type,
    const blockchain::filter::Hash& hash,
    std::unique_ptr<blockchain::internal::GCS> filter)
    -> blockchain::p2p::bitcoin::message::internal::Cfilter*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Cfilter;

    OT_ASSERT(filter);

    return new ReturnType(
        api, network, type, hash, filter->ElementCount(), filter->Compressed());
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
const std::map<filter::Type, std::uint8_t> Cfilter::gcs_bits_{
    {filter::Type::Basic_BIP158, 19},
    {filter::Type::Basic_BCHVariant, 19},
    {filter::Type::Extended_opentxs, 19},
};
const std::map<filter::Type, std::uint32_t> Cfilter::gcs_fp_rate_{
    {filter::Type::Basic_BIP158, 784931},
    {filter::Type::Basic_BCHVariant, 784931},
    {filter::Type::Extended_opentxs, 784931},
};

Cfilter::Cfilter(
    const api::client::Manager& api,
    const blockchain::Type network,
    const filter::Type type,
    const filter::Hash& hash,
    const std::uint32_t count,
    const Space& compressed) noexcept
    : Message(api, network, bitcoin::Command::cfilter)
    , type_(type)
    , hash_(hash)
    , count_(count)
    , filter_(compressed)
{
    init_hash();
}

Cfilter::Cfilter(
    const api::client::Manager& api,
    std::unique_ptr<Header> header,
    const filter::Type type,
    const filter::Hash& hash,
    const std::uint32_t count,
    Space&& compressed) noexcept
    : Message(api, std::move(header))
    , type_(type)
    , hash_(hash)
    , count_(count)
    , filter_(std::move(compressed))
{
}

auto Cfilter::payload() const noexcept -> OTData
{
    try {
        auto raw = BitcoinFormat{header().Network(), type_, hash_};
        auto output = Data::Factory(&raw, sizeof(raw));
        auto filter = CompactSize(count_).Encode();
        filter.insert(filter.end(), filter_.begin(), filter_.end());
        output->Concatenate(filter.data(), filter.size());

        return output;
    } catch (...) {
        return Data::Factory();
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
