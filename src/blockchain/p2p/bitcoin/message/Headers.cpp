// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include "blockchain/bitcoin/CompactSize.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"

#include "Headers.hpp"

//#define OT_METHOD " opentxs::blockchain::p2p::bitcoin::message::Headers::"

namespace opentxs
{
blockchain::p2p::bitcoin::message::internal::Headers* Factory::
    BitcoinP2PHeaders(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Headers;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid header")
            .Flush();

        return nullptr;
    }

    auto expectedSize = sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Payload too short (compactsize)")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    auto count = std::size_t{0};
    const bool decodedSize = blockchain::bitcoin::DecodeCompactSizeFromPayload(
        it, expectedSize, size, count);

    if (false == decodedSize) {
        LogOutput(__FUNCTION__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    std::vector<std::unique_ptr<blockchain::block::bitcoin::Header>> headers{};

    if (count > 0) {
        for (std::size_t i{0}; i < count; ++i) {
            expectedSize += 81;

            if (expectedSize > size) {
                LogOutput("opentxs::Factory::")(__FUNCTION__)(
                    ": Block Header entries incomplete at entry index ")(i)
                    .Flush();

                return nullptr;
            }

            std::unique_ptr<blockchain::block::bitcoin::Header> pHeader{
                Factory::BitcoinBlockHeader(api, Data::Factory(it, 80))};

            OT_ASSERT(pHeader);

            headers.emplace_back(std::move(pHeader));

            it += 81;
        }
    }

    return new ReturnType(api, std::move(pHeader), std::move(headers));
}

blockchain::p2p::bitcoin::message::internal::Headers* Factory::
    BitcoinP2PHeaders(
        const api::internal::Core& api,
        const blockchain::Type network,
        std::vector<std::unique_ptr<blockchain::block::bitcoin::Header>>&&
            headers)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Headers;

    return new ReturnType(api, network, std::move(headers));
}
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Headers::Headers(
    const api::internal::Core& api,
    const blockchain::Type network,
    std::vector<std::unique_ptr<value_type>>&& headers) noexcept
    : Message(api, network, bitcoin::Command::headers)
    , payload_(std::move(headers))
{
    init_hash();
}

Headers::Headers(
    const api::internal::Core& api,
    std::unique_ptr<Header> header,
    std::vector<std::unique_ptr<value_type>>&& headers) noexcept
    : Message(api, std::move(header))
    , payload_(std::move(headers))
{
}

OTData Headers::payload() const noexcept
{
    const auto null = Data::Factory("0x00", Data::Mode::Hex);
    auto output = Data::Factory(CompactSize(payload_.size()).Encode());

    for (const auto& pHeader : payload_) {
        OT_ASSERT(pHeader);

        const auto& header = *pHeader;
        output += header.Encode();
        output += null;
    }

    return output;
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
