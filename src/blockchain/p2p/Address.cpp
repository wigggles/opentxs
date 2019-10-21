// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#include "internal/api/Api.hpp"
#include "internal/blockchain/p2p/P2P.hpp"

#include "Address.hpp"

// #define OT_METHOD "opentxs::blockchain::p2p::implementation::Address::"

namespace opentxs
{
blockchain::p2p::internal::Address* Factory::BlockchainAddress(
    const api::internal::Core& api,
    const blockchain::p2p::Protocol protocol,
    const blockchain::p2p::Network network,
    const Data& bytes,
    const std::uint16_t port,
    const blockchain::Type chain,
    const Time lastConnected,
    const std::set<blockchain::p2p::Service>& services)
{
    using ReturnType = blockchain::p2p::implementation::Address;

    try {
        return new ReturnType(
            api,
            protocol,
            network,
            bytes,
            port,
            chain,
            lastConnected,
            services);
    } catch (...) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Invalid bytes")
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs

namespace opentxs::blockchain::p2p::implementation
{
Address::Address(
    const api::internal::Core& api,
    const Protocol protocol,
    const Network network,
    const Data& bytes,
    const std::uint16_t port,
    const blockchain::Type chain,
    const Time lastConnected,
    const std::set<Service>& services) noexcept(false)
    : api_(api)
    , id_(calculate_id(bytes, port))
    , protocol_(protocol)
    , network_(network)
    , bytes_(bytes)
    , port_(port)
    , chain_(chain)
    , last_connected_(lastConnected)
    , services_(services)
{
    const auto size = bytes_->size();

    switch (network_) {
        case Network::ipv4: {
            if (sizeof(ip::address_v4::bytes_type) != size) {
                throw std::runtime_error("Incorrect ipv4 bytes");
            }
        } break;
        case Network::ipv6:
        case Network::cjdns: {
            if (sizeof(ip::address_v6::bytes_type) != size) {
                throw std::runtime_error("Incorrect ipv6 bytes");
            }
        } break;
        case Network::onion2: {
            if (10 != size) {
                throw std::runtime_error("Incorrect onion bytes");
            }
        } break;
        case Network::onion3: {
            if (56 != size) {
                throw std::runtime_error("Incorrect onion bytes");
            }
        } break;
        case Network::eep: {
            if (32 != size) {  // TODO replace ths with correct value
                throw std::runtime_error("Incorrect eep bytes");
            }
        } break;
        default: {
            OT_FAIL;
        }
    }
}

Address::Address(const Address& rhs) noexcept
    : api_(rhs.api_)
    , id_(rhs.id_)
    , protocol_(rhs.protocol_)
    , network_(rhs.network_)
    , bytes_(rhs.bytes_)
    , port_(rhs.port_)
    , chain_(rhs.chain_)
    , last_connected_(rhs.last_connected_)
    , services_(rhs.services_)
{
}

OTIdentifier Address::calculate_id(
    const Data& bytes,
    const std::uint16_t port) noexcept
{
    OTData preimage(bytes);
    preimage->Concatenate(&port, sizeof(port));
    auto output = Identifier::Factory();
    output->CalculateDigest(preimage);

    return output;
}

std::string Address::Display() const noexcept
{
    std::string output{};

    switch (network_) {
        case Network::ipv4: {
            ip::address_v4::bytes_type bytes{};
            std::memcpy(bytes.data(), bytes_->data(), bytes.size());
            auto address = ip::make_address_v4(bytes);
            output = address.to_string();
        } break;
        case Network::ipv6:
        case Network::cjdns: {
            ip::address_v6::bytes_type bytes{};
            std::memcpy(bytes.data(), bytes_->data(), bytes.size());
            auto address = ip::make_address_v6(bytes);
            output = std::string("[") + address.to_string() + "]";
        } break;
        case Network::onion2:
        case Network::onion3: {
            output =
                std::string(
                    static_cast<const char*>(bytes_->data()), bytes_->size()) +
                ".onion";
        } break;
        case Network::eep: {
            output = api_.Crypto().Encode().DataEncode(bytes_) + ".i2p";
        } break;
        default: {
            OT_FAIL;
        }
    }

    return output + ":" + std::to_string(port_);
}
}  // namespace opentxs::blockchain::p2p::implementation
