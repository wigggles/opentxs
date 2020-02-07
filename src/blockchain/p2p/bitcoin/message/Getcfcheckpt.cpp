// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"

#include "Getcfcheckpt.hpp"

// #define OT_METHOD
// "opentxs::blockchain::p2p::bitcoin::message::implemenetationGetcfcheckpt::"

namespace opentxs
{
blockchain::p2p::bitcoin::message::internal::Getcfcheckpt* Factory::
    BitcoinP2PGetcfcheckpt(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
        const blockchain::p2p::bitcoin::ProtocolVersion version,
        const void* payload,
        const std::size_t size)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Getcfcheckpt;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid header")
            .Flush();

        return nullptr;
    }

    ReturnType::BitcoinFormat raw;
    auto expectedSize = sizeof(raw);

    if (expectedSize > size) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Payload too short")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    std::memcpy(reinterpret_cast<std::byte*>(&raw), it, sizeof(raw));
    it += sizeof(raw);

    return new ReturnType(api, std::move(pHeader), raw.Type(), raw.Hash());
}

blockchain::p2p::bitcoin::message::internal::Getcfcheckpt* Factory::
    BitcoinP2PGetcfcheckpt(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::filter::Type type,
        const blockchain::filter::Hash& stop)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Getcfcheckpt;

    return new ReturnType(api, network, type, stop);
}
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Getcfcheckpt::Getcfcheckpt(
    const api::internal::Core& api,
    const blockchain::Type network,
    const filter::Type type,
    const filter::Hash& stop) noexcept
    : Message(api, network, bitcoin::Command::getcfcheckpt)
    , type_(type)
    , stop_(stop)
{
    init_hash();
}

Getcfcheckpt::Getcfcheckpt(
    const api::internal::Core& api,
    std::unique_ptr<Header> header,
    const filter::Type type,
    const filter::Hash& stop) noexcept
    : Message(api, std::move(header))
    , type_(type)
    , stop_(stop)
{
}

OTData Getcfcheckpt::payload() const noexcept
{
    try {
        BitcoinFormat raw(type_, stop_);
        auto output = Data::Factory(&raw, sizeof(raw));

        return output;
    } catch (...) {
        return Data::Factory();
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
