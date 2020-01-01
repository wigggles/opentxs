// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include <opentxs/blockchain/p2p/Address.hpp>
#include "opentxs/blockchain/p2p/Peer.hpp"
#include "opentxs/blockchain/Blockchain.hpp"

#include <boost/asio.hpp>

#include "core/StateMachine.hpp"

#include <cstdint>
#include <future>
#include <set>

namespace ba = boost::asio;
namespace ip = ba::ip;
using tcp = ip::tcp;

namespace opentxs::blockchain::p2p::internal
{
struct Address : virtual public p2p::Address {
    virtual std::unique_ptr<Address> clone_internal() const noexcept = 0;
    virtual Time PreviousLastConnected() const noexcept = 0;
    virtual std::set<Service> PreviousServices() const noexcept = 0;

    ~Address() override = default;
};

struct Peer : virtual public p2p::Peer {
    virtual OTIdentifier AddressID() const noexcept = 0;
    virtual std::shared_future<void> Shutdown() noexcept = 0;

    virtual ~Peer() override = default;
};
}  // namespace opentxs::blockchain::p2p::internal
