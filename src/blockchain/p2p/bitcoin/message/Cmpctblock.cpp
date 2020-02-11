// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"

#include "Cmpctblock.hpp"

//#define OT_METHOD " opentxs::blockchain::p2p::bitcoin::message::Cmpctblock::"

namespace opentxs
{
// We have a header and a raw payload. Parse it.
blockchain::p2p::bitcoin::message::Cmpctblock* Factory::BitcoinP2PCmpctblock(
    const api::internal::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Cmpctblock;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid header")
            .Flush();

        return nullptr;
    }

    const auto raw_cmpctblock(Data::Factory(payload, size));
    // --------------------------------------------------------
    try {
        return new ReturnType(api, std::move(pHeader), raw_cmpctblock);
    } catch (...) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Checksum failure")
            .Flush();

        return nullptr;
    }
}

// We have all the data members to create the message from scratch (for sending)
blockchain::p2p::bitcoin::message::Cmpctblock* Factory::BitcoinP2PCmpctblock(
    const api::internal::Core& api,
    const blockchain::Type network,
    const Data& raw_cmpctblock)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Cmpctblock;

    return new ReturnType(api, network, raw_cmpctblock);
}
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message
{

OTData Cmpctblock::payload() const noexcept
{
    try {
        return getRawCmpctblock();
    } catch (...) {
        return Data::Factory();
    }
}

// We have all the data members to create the message from scratch (for sending)
Cmpctblock::Cmpctblock(
    const api::internal::Core& api,
    const blockchain::Type network,
    const Data& raw_cmpctblock) noexcept
    : Message(api, network, bitcoin::Command::cmpctblock)
    , raw_cmpctblock_(Data::Factory(raw_cmpctblock))
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Cmpctblock::Cmpctblock(
    const api::internal::Core& api,
    std::unique_ptr<Header> header,
    const Data& raw_cmpctblock) noexcept(false)
    : Message(api, std::move(header))
    , raw_cmpctblock_(Data::Factory(raw_cmpctblock))
{
    verify_checksum();
}

}  // namespace  opentxs::blockchain::p2p::bitcoin::message
