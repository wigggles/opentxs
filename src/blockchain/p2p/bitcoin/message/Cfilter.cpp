// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/blockchain/Blockchain.hpp"

#include "Cfilter.hpp"

// #define OT_METHOD "opentxs::blockchain::p2p::bitcoin::message::Cfilter::"
namespace opentxs
{
blockchain::p2p::bitcoin::message::internal::Cfilter* Factory::
    BitcoinP2PCfilter(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Cfilter;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid header")
            .Flush();

        return nullptr;
    }

    auto raw = ReturnType::BitcoinFormat{};
    auto expectedSize = sizeof(raw);

    if (expectedSize > size) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Payload too short (begin)")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    std::memcpy(reinterpret_cast<std::byte*>(&raw), it, sizeof(raw));
    std::advance(it, sizeof(raw));
    expectedSize += 1;

    if (expectedSize > size) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
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
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
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

    const auto filterType = raw.Type();
    const auto dataSize = filterSize - (1 + csBytes);
    auto gcs = std::unique_ptr<blockchain::internal::GCS>{};
    auto bytes = Data::Factory(it, dataSize);

    try {
        gcs.reset(Factory::GCS(
            api,
            ReturnType::gcs_bits_.at(filterType),
            ReturnType::gcs_fp_rate_.at(filterType),
            blockchain::internal::BlockHashToFilterKey(raw.Hash()->Bytes()),
            elementCount,
            std::move(bytes)));
    } catch (...) {
        LogOutput(__FUNCTION__)(": Unknown filter type").Flush();

        return nullptr;
    }

    return new ReturnType(
        api, std::move(pHeader), raw.Type(), raw.Hash(), std::move(gcs));
}

blockchain::p2p::bitcoin::message::internal::Cfilter* Factory::
    BitcoinP2PCfilter(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::filter::Type type,
        const blockchain::filter::Hash& hash,
        std::unique_ptr<blockchain::internal::GCS> filter)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Cfilter;

    return new ReturnType(api, network, type, hash, std::move(filter));
}
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
const std::map<filter::Type, std::uint32_t> Cfilter::gcs_bits_{
    {filter::Type::Basic, 19},
};
const std::map<filter::Type, std::uint32_t> Cfilter::gcs_fp_rate_{
    {filter::Type::Basic, 784931},
};

Cfilter::Cfilter(
    const api::internal::Core& api,
    const blockchain::Type network,
    const filter::Type type,
    const filter::Hash& hash,
    std::unique_ptr<blockchain::internal::GCS> filter) noexcept
    : Message(api, network, bitcoin::Command::cfilter)
    , type_(type)
    , hash_(hash)
    , filter_(std::move(filter))
{
    OT_ASSERT(filter_);

    init_hash();
}

Cfilter::Cfilter(
    const api::internal::Core& api,
    std::unique_ptr<Header> header,
    const filter::Type type,
    const filter::Hash& hash,
    std::unique_ptr<blockchain::internal::GCS> filter) noexcept
    : Message(api, std::move(header))
    , type_(type)
    , hash_(hash)
    , filter_(std::move(filter))
{
    OT_ASSERT(filter_);
}

OTData Cfilter::payload() const noexcept
{
    try {
        BitcoinFormat raw(type_, hash_);
        auto output = Data::Factory(&raw, sizeof(raw));
        auto bytes = filter_->Encode();
        const auto size = CompactSize(bytes->size()).Encode();
        output->Concatenate(size.data(), size.size());
        output += bytes;

        return output;
    } catch (...) {
        return Data::Factory();
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
