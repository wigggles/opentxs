// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include "blockchain/bitcoin/CompactSize.hpp"
#include "blockchain/bitcoin/Inventory.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"

#include <vector>

#include "Getdata.hpp"

//#define OT_METHOD "
// opentxs::blockchain::p2p::bitcoin::message::implementation::Getdata::"

namespace opentxs
{
blockchain::p2p::bitcoin::message::internal::Getdata* Factory::
    BitcoinP2PGetdata(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size)
{
    namespace bitcoin = blockchain::p2p::bitcoin::message;
    using ReturnType = bitcoin::implementation::Getdata;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid header")
            .Flush();

        return nullptr;
    }

    auto expectedSize = sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Size below minimum for Getdata 1")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    std::size_t count{0};
    const bool haveCount = blockchain::bitcoin::DecodeCompactSizeFromPayload(
        it, expectedSize, size, count);

    if (false == haveCount) {
        LogOutput(__FUNCTION__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    std::vector<blockchain::bitcoin::Inventory> items{};

    if (count > 0) {
        for (std::size_t i{0}; i < count; ++i) {
            expectedSize += ReturnType::value_type::EncodedSize;

            if (expectedSize > size) {
                LogOutput("opentxs::Factory::")(__FUNCTION__)(
                    ": Inventory entries incomplete at entry index ")(i)
                    .Flush();

                return nullptr;
            }

            items.emplace_back(it, ReturnType::value_type::EncodedSize);
            it += ReturnType::value_type::EncodedSize;
        }
    }

    return new ReturnType(api, std::move(pHeader), std::move(items));
}

blockchain::p2p::bitcoin::message::internal::Getdata* Factory::
    BitcoinP2PGetdata(
        const api::internal::Core& api,
        const blockchain::Type network,
        std::vector<blockchain::bitcoin::Inventory>&& payload)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Getdata;

    return new ReturnType(api, network, std::move(payload));
}
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Getdata::Getdata(
    const api::internal::Core& api,
    const blockchain::Type network,
    std::vector<blockchain::bitcoin::Inventory>&& payload) noexcept
    : Message(api, network, bitcoin::Command::getdata)
    , payload_(std::move(payload))
{
    init_hash();
}

Getdata::Getdata(
    const api::internal::Core& api,
    std::unique_ptr<Header> header,
    std::vector<blockchain::bitcoin::Inventory>&& payload) noexcept
    : Message(api, std::move(header))
    , payload_(std::move(payload))
{
}

OTData Getdata::payload() const noexcept
{
    try {
        auto output = Data::Factory(CompactSize(payload_.size()).Encode());

        for (const auto& item : payload_) { output += item.Encode(); }

        return output;
    } catch (...) {

        return Data::Factory();
    }
}
}  // namespace  opentxs::blockchain::p2p::bitcoin::message::implementation
