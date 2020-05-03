// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Sendcmpct.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <utility>

#include "Factory.hpp"
#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"

//#define OT_METHOD " opentxs::blockchain::p2p::bitcoin::message::Sendcmpct::"

namespace opentxs
{
// We have a header and a raw payload. Parse it.
blockchain::p2p::bitcoin::message::Sendcmpct* Factory::BitcoinP2PSendcmpct(
    const api::internal::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion,
    const void* payload,
    const std::size_t size)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Sendcmpct;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid header")
            .Flush();

        return nullptr;
    }

    auto expectedSize = sizeof(ReturnType::Raw);

    if (expectedSize > size) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Size below minimum for Sendcmpct 1")
            .Flush();

        return nullptr;
    }
    auto* it{static_cast<const std::byte*>(payload)};
    // --------------------------------------------------------
    ReturnType::Raw raw_item;

    OTPassword::safe_memcpy(&raw_item, sizeof(raw_item), it, sizeof(raw_item));
    it += sizeof(raw_item);

    const bool announce = (0 == raw_item.announce_.value()) ? false : true;
    const std::uint64_t version = raw_item.version_.value();
    // --------------------------------------------------------
    try {
        return new ReturnType(api, std::move(pHeader), announce, version);
    } catch (...) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Checksum failure")
            .Flush();

        return nullptr;
    }
}

// We have all the data members to create the message from scratch (for sending)
blockchain::p2p::bitcoin::message::Sendcmpct* Factory::BitcoinP2PSendcmpct(
    const api::internal::Core& api,
    const blockchain::Type network,
    const bool announce,
    const std::uint64_t version)
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Sendcmpct;

    return new ReturnType(api, network, announce, version);
}
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message
{
// We have all the data members to create the message from scratch (for sending)
Sendcmpct::Sendcmpct(
    const api::internal::Core& api,
    const blockchain::Type network,
    const bool announce,
    const std::uint64_t version) noexcept
    : Message(api, network, bitcoin::Command::sendcmpct)
    , announce_(announce)
    , version_(version)
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Sendcmpct::Sendcmpct(
    const api::internal::Core& api,
    std::unique_ptr<Header> header,
    const bool announce,
    const std::uint64_t version) noexcept(false)
    : Message(api, std::move(header))
    , announce_(announce)
    , version_(version)
{
    verify_checksum();
}

Sendcmpct::Raw::Raw(bool announce, std::uint64_t version) noexcept
    : announce_(static_cast<std::int8_t>(announce ? 1 : 0))
    , version_(version)
{
}

Sendcmpct::Raw::Raw() noexcept
    : announce_(0)
    , version_(0)
{
}

OTData Sendcmpct::payload() const noexcept
{
    try {
        Sendcmpct::Raw raw_item(announce_, version_);
        auto output = Data::Factory(&raw_item, sizeof(raw_item));
        return output;
    } catch (...) {
        return Data::Factory();
    }
}
}  // namespace  opentxs::blockchain::p2p::bitcoin::message
