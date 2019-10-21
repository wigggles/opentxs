// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/network/zeromq/Frame.hpp"

#include "blockchain/p2p/bitcoin/Header.hpp"

#include <boost/endian/arithmetic.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/endian/buffers.hpp>

#define HEADER_SIZE 24

//#define OT_METHOD "opentxs::blockchain::p2p::bitcoin::MessageHeader::"

namespace opentxs
{
blockchain::p2p::bitcoin::Header* Factory::BitcoinP2PHeader(
    const api::internal::Core& api,
    const network::zeromq::Frame& bytes)
{
    using ReturnType = opentxs::blockchain::p2p::bitcoin::Header;
    const ReturnType::BitcoinFormat raw{bytes};

    return new ReturnType(
        api,
        GetNetwork(raw.Magic()),
        raw.Command(),
        raw.PayloadSize(),
        raw.Checksum());
}
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin
{
Header::Header(
    const api::internal::Core& api,
    const blockchain::Type network,
    const bitcoin::Command command,
    const std::size_t payload,
    const OTData checksum) noexcept
    : chain_(network)
    , command_(command)
    , payload_size_(payload)
    , checksum_(checksum)
{
}

Header::Header(
    const api::internal::Core& api,
    const blockchain::Type network,
    const bitcoin::Command command) noexcept
    : Header(api, network, command, 0, Data::Factory())
{
}

Header::BitcoinFormat::BitcoinFormat(
    const void* data,
    const std::size_t size) noexcept(false)
    : magic_()
    , command_()
    , length_()
    , checksum_()
{
    static_assert(HEADER_SIZE == sizeof(BitcoinFormat));

    if (sizeof(BitcoinFormat) != size) {
        throw std::invalid_argument("Incorrect input size");
    }

    std::memcpy(this, data, size);
}

Header::BitcoinFormat::BitcoinFormat(
    const blockchain::Type network,
    const bitcoin::Command command,
    const std::size_t payload,
    const OTData checksum) noexcept(false)
    : magic_(static_cast<std::uint32_t>(GetMagic(network)))
    , command_(SerializeCommand(command))
    , length_(payload)
    , checksum_()
{
    static_assert(HEADER_SIZE == sizeof(BitcoinFormat));

    if (sizeof(checksum_) != checksum->size()) {
        throw std::invalid_argument("Incorrect checksum size");
    }

    std::memcpy(checksum_.data(), checksum->data(), checksum->size());
}

Header::BitcoinFormat::BitcoinFormat(const Data& in) noexcept(false)
    : BitcoinFormat(in.data(), in.size())
{
}

Header::BitcoinFormat::BitcoinFormat(const zmq::Frame& in) noexcept(false)
    : BitcoinFormat(in.data(), in.size())
{
}

OTData Header::BitcoinFormat::Checksum() const noexcept
{
    return Data::Factory(checksum_.data(), checksum_.size());
}

bitcoin::Command Header::BitcoinFormat::Command() const noexcept
{
    return GetCommand(command_);
}

bitcoin::Magic Header::BitcoinFormat::Magic() const noexcept
{
    return GetMagic(magic_.value());
}

std::size_t Header::BitcoinFormat::PayloadSize() const noexcept
{
    return length_.value();
}

OTData Header::Encode() const noexcept
{
    auto output = Data::Factory();

    try {
        const auto raw =
            BitcoinFormat(chain_, command_, payload_size_, checksum_);
        output->Assign(&raw, sizeof(raw));
    } catch (...) {
    }

    return output;
}

void Header::SetChecksum(const std::size_t payload, OTData&& checksum) noexcept
{
    payload_size_ = payload;
    checksum_ = std::move(checksum);
}
}  // namespace opentxs::blockchain::p2p::bitcoin
