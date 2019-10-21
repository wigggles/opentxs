// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::blockchain::p2p::implementation
{
class Address final : public internal::Address
{
public:
    OTData Bytes() const noexcept final { return bytes_; }
    blockchain::Type Chain() const noexcept final { return chain_; }
    std::string Display() const noexcept final;
    const Identifier& ID() const noexcept final { return id_; }
    Time LastConnected() const noexcept final { return last_connected_; }
    std::uint16_t Port() const noexcept final { return port_; }
    std::set<Service> Services() const noexcept final { return services_; }
    Protocol Style() const noexcept final { return protocol_; }
    Network Type() const noexcept final { return network_; }

    void AddService(const Service service) noexcept final
    {
        services_.emplace(service);
    }
    void RemoveService(const Service service) noexcept final
    {
        services_.erase(service);
    }
    void SetLastConnected(const Time& time) noexcept final
    {
        last_connected_ = time;
    }
    void SetServices(const std::set<Service>& services) noexcept final
    {
        services_ = services;
    }

    Address(const Address& rhs) noexcept;

    ~Address() final = default;

private:
    friend opentxs::Factory;

    const api::internal::Core& api_;
    const OTIdentifier id_;
    const Protocol protocol_;
    const Network network_;
    const OTData bytes_;
    const std::uint16_t port_;
    const blockchain::Type chain_;
    Time last_connected_;
    std::set<Service> services_;

    static OTIdentifier calculate_id(
        const Data& bytes,
        const std::uint16_t port) noexcept;

    Address* clone() const noexcept final { return new Address(*this); }
    std::unique_ptr<internal::Address> clone_internal() const noexcept final
    {
        return std::make_unique<Address>(*this);
    }

    Address(
        const api::internal::Core& api,
        const Protocol protocol,
        const Network network,
        const Data& bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const std::set<Service>& services) noexcept(false);
    Address() = delete;
    Address(Address&&) = delete;
    Address& operator=(const Address&) = delete;
    Address& operator=(Address&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::implementation
