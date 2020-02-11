// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"

#include "Tx.hpp"

//#define OT_METHOD " opentxs::blockchain::p2p::bitcoin::message::Tx::"

namespace opentxs
{
// We have a header and a raw payload. Parse it.
blockchain::p2p::bitcoin::message::Tx* Factory::BitcoinP2PTx(
    const api::internal::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Tx;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid header")
            .Flush();

        return nullptr;
    }

    const auto raw_tx(Data::Factory(payload, size));
    // --------------------------------------------------------
    try {
        return new ReturnType(api, std::move(pHeader), raw_tx);
    } catch (...) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Checksum failure")
            .Flush();

        return nullptr;
    }
}

// We have all the data members to create the message from scratch (for sending)
blockchain::p2p::bitcoin::message::Tx* Factory::BitcoinP2PTx(
    const api::internal::Core& api,
    const blockchain::Type network,
    const Data& raw_tx)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Tx;

    return new ReturnType(api, network, raw_tx);
}
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message
{

OTData Tx::payload() const noexcept
{
    try {
        return getRawTx();
    } catch (...) {
        return Data::Factory();
    }
}

// We have all the data members to create the message from scratch (for sending)
Tx::Tx(
    const api::internal::Core& api,
    const blockchain::Type network,
    const Data& raw_tx) noexcept
    : Message(api, network, bitcoin::Command::tx)
    , raw_tx_(Data::Factory(raw_tx))
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Tx::Tx(
    const api::internal::Core& api,
    std::unique_ptr<Header> header,
    const Data& raw_tx) noexcept(false)
    : Message(api, std::move(header))
    , raw_tx_(Data::Factory(raw_tx))
{
    verify_checksum();
}

}  // namespace  opentxs::blockchain::p2p::bitcoin::message
