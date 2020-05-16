// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Version.cpp"

#pragma once

#include <memory>
#include <set>
#include <string>

#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain

class Factory;
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Version final : public internal::Version
{
public:
    auto Height() const noexcept -> block::Height final { return height_; }
    auto LocalAddress() const noexcept -> tcp::endpoint final
    {
        return local_address_;
    }
    auto LocalServices() const noexcept
        -> std::set<blockchain::p2p::Service> final
    {
        return local_services_;
    }
    auto Nonce() const noexcept -> api::client::blockchain::Nonce final
    {
        return nonce_;
    }
    auto ProtocolVersion() const noexcept -> bitcoin::ProtocolVersion final
    {
        return version_;
    }
    auto Relay() const noexcept -> bool final { return relay_; }
    auto RemoteAddress() const noexcept -> tcp::endpoint final
    {
        return remote_address_;
    }
    auto RemoteServices() const noexcept
        -> std::set<blockchain::p2p::Service> final
    {
        return remote_services_;
    }
    auto UserAgent() const noexcept -> const std::string& final
    {
        return user_agent_;
    }

    ~Version() final = default;

private:
    friend opentxs::Factory;

    struct BitcoinFormat_1 {
        ProtocolVersionFieldSigned version_{};
        BitVectorField services_{};
        TimestampField64 timestamp_{};
        AddressVersion remote_{};

        BitcoinFormat_1(
            const bitcoin::ProtocolVersion version,
            const std::set<bitcoin::Service>& localServices,
            const std::set<bitcoin::Service>& remoteServices,
            const tcp::endpoint& remoteAddress,
            const Time time) noexcept;
        BitcoinFormat_1() noexcept;
    };

    struct BitcoinFormat_106 {
        AddressVersion local_{};
        NonceField nonce_{};

        BitcoinFormat_106(
            const std::set<bitcoin::Service>& localServices,
            const tcp::endpoint localAddress,
            const api::client::blockchain::Nonce nonce) noexcept;
        BitcoinFormat_106() noexcept;
    };

    struct BitcoinFormat_209 {
        HeightField height_{};

        BitcoinFormat_209(const block::Height height) noexcept;
        BitcoinFormat_209() noexcept;
    };

    const bitcoin::ProtocolVersion version_;
    const tcp::endpoint local_address_;
    const tcp::endpoint remote_address_;
    const std::set<blockchain::p2p::Service> services_;
    const std::set<blockchain::p2p::Service> local_services_;
    const std::set<blockchain::p2p::Service> remote_services_;
    const api::client::blockchain::Nonce nonce_;
    const std::string user_agent_;
    const block::Height height_;
    const bool relay_;
    const Time timestamp_;

    auto payload() const noexcept -> OTData final;

    Version(
        const api::internal::Core& api,
        const blockchain::Type network,
        const bitcoin::ProtocolVersion version,
        const tcp::endpoint localAddress,
        const tcp::endpoint remoteAddress,
        const std::set<blockchain::p2p::Service>& services,
        const std::set<blockchain::p2p::Service>& localServices,
        const std::set<blockchain::p2p::Service>& remoteServices,
        const api::client::blockchain::Nonce nonce,
        const std::string& userAgent,
        const block::Height height,
        const bool relay,
        const Time time = Clock::now()) noexcept;
    Version(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const bitcoin::ProtocolVersion version,
        const tcp::endpoint localAddress,
        const tcp::endpoint remoteAddress,
        const std::set<blockchain::p2p::Service>& services,
        const std::set<blockchain::p2p::Service>& localServices,
        const std::set<blockchain::p2p::Service>& remoteServices,
        const api::client::blockchain::Nonce nonce,
        const std::string& userAgent,
        const block::Height height,
        const bool relay,
        const Time time = Clock::now()) noexcept;
    Version(const Version&) = delete;
    Version(Version&&) = delete;
    auto operator=(const Version&) -> Version& = delete;
    auto operator=(Version &&) -> Version& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
