// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"

#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <tuple>

namespace ot = opentxs;
namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::bitcoin
{
class Header final
{
public:
    struct BitcoinFormat {
        MagicField magic_;
        CommandField command_;
        PayloadSizeField length_;
        ChecksumField checksum_;

        OTData Checksum() const noexcept;
        bitcoin::Command Command() const noexcept;
        bitcoin::Magic Magic() const noexcept;
        std::size_t PayloadSize() const noexcept;

        BitcoinFormat(const Data& in) noexcept(false);
        BitcoinFormat(const zmq::Frame& in) noexcept(false);
        BitcoinFormat(
            const blockchain::Type network,
            const bitcoin::Command command,
            const std::size_t payload,
            const OTData checksum) noexcept(false);

    private:
        BitcoinFormat(const void* data, const std::size_t size) noexcept(false);
    };

    static std::size_t Size() noexcept { return sizeof(BitcoinFormat); }

    bitcoin::Command Command() const noexcept { return command_; }
    OPENTXS_EXPORT OTData Encode() const noexcept;
    blockchain::Type Network() const noexcept { return chain_; }
    std::size_t PayloadSize() const noexcept { return payload_size_; }
    const opentxs::Data& Checksum() const noexcept { return checksum_; }

    void SetChecksum(const std::size_t payload, OTData&& checksum) noexcept;

    Header(
        const api::internal::Core& api,
        const blockchain::Type network,
        const bitcoin::Command command) noexcept;

    ~Header() = default;

private:
    friend opentxs::Factory;

    blockchain::Type chain_{};
    bitcoin::Command command_{};
    std::size_t payload_size_{};
    OTData checksum_;

    Header(
        const api::internal::Core& api,
        const blockchain::Type network,
        const bitcoin::Command command,
        const std::size_t payload,
        const OTData checksum) noexcept;
    Header() = delete;
    Header(const Header&) = delete;
    Header(Header&&) = delete;
    Header& operator=(const Header&) = delete;
    Header& operator=(Header&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin
