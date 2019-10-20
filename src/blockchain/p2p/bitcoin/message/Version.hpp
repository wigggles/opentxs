// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Version final : public internal::Version
{
public:
    block::Height Height() const noexcept final { return height_; }
    tcp::endpoint LocalAddress() const noexcept final { return local_address_; }
    std::set<blockchain::p2p::Service> LocalServices() const noexcept final
    {
        return local_services_;
    }
    api::client::blockchain::Nonce Nonce() const noexcept final
    {
        return nonce_;
    }
    bitcoin::ProtocolVersion ProtocolVersion() const noexcept final
    {
        return version_;
    }
    bool Relay() const noexcept final { return relay_; }
    tcp::endpoint RemoteAddress() const noexcept final
    {
        return remote_address_;
    }
    std::set<blockchain::p2p::Service> RemoteServices() const noexcept final
    {
        return remote_services_;
    }
    const std::string& UserAgent() const noexcept final { return user_agent_; }

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

    OTData payload() const noexcept final;

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
    Version& operator=(const Version&) = delete;
    Version& operator=(Version&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
