// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/blockchain/BloomFilter.hpp"

#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"

#include <boost/endian/buffers.hpp>

#include <set>

#include "Filterload.hpp"

// #define OT_METHOD
// "opentxs::blockchain::p2p::bitcoin::message::implementation::Filterload::"

namespace opentxs
{
blockchain::p2p::bitcoin::message::internal::Filterload* Factory::
    BitcoinP2PFilterload(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Filterload;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid header")
            .Flush();

        return nullptr;
    }

    std::unique_ptr<blockchain::BloomFilter> pFilter{
        Factory::BloomFilter(api, Data::Factory(payload, size))};

    if (false == bool(pFilter)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid filter")
            .Flush();

        return nullptr;
    }

    return new ReturnType(api, std::move(pHeader), *pFilter);
}

blockchain::p2p::bitcoin::message::internal::Filterload* Factory::
    BitcoinP2PFilterload(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::BloomFilter& filter)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Filterload;

    return new ReturnType(api, network, filter);
}
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Filterload::Filterload(
    const api::internal::Core& api,
    const blockchain::Type network,
    const blockchain::BloomFilter& filter) noexcept
    : Message(api, network, bitcoin::Command::filterload)
    , payload_(filter)
{
    init_hash();
}

Filterload::Filterload(
    const api::internal::Core& api,
    std::unique_ptr<Header> header,
    const blockchain::BloomFilter& filter) noexcept
    : Message(api, std::move(header))
    , payload_(filter)
{
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
