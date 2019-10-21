// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_P2P_ADDRESS_HPP
#define OPENTXS_BLOCKCHAIN_P2P_ADDRESS_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/blockchain/Blockchain.hpp"

#include <set>

namespace opentxs
{
namespace blockchain
{
namespace p2p
{
class Address
{
public:
    EXPORT virtual OTData Bytes() const noexcept = 0;
    EXPORT virtual blockchain::Type Chain() const noexcept = 0;
    EXPORT virtual std::string Display() const noexcept = 0;
    EXPORT virtual const Identifier& ID() const noexcept = 0;
    EXPORT virtual Time LastConnected() const noexcept = 0;
    EXPORT virtual std::uint16_t Port() const noexcept = 0;
    EXPORT virtual std::set<Service> Services() const noexcept = 0;
    EXPORT virtual Protocol Style() const noexcept = 0;
    EXPORT virtual Network Type() const noexcept = 0;

    EXPORT virtual void AddService(const Service service) noexcept = 0;
    EXPORT virtual void RemoveService(const Service service) noexcept = 0;
    EXPORT virtual void SetLastConnected(const Time& time) noexcept = 0;
    EXPORT virtual void SetServices(
        const std::set<Service>& services) noexcept = 0;

    EXPORT virtual ~Address() = default;

protected:
    Address() noexcept = default;

private:
    friend OTBlockchainAddress;

    virtual Address* clone() const noexcept = 0;

    Address(const Address&) = delete;
    Address(Address&&) = delete;
    Address& operator=(const Address&) = delete;
    Address& operator=(Address&&) = delete;
};
}  // namespace p2p
}  // namespace blockchain
}  // namespace opentxs
#endif
