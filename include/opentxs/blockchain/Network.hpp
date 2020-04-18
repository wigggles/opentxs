// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_NETWORK_HPP
#define OPENTXS_BLOCKCHAIN_NETWORK_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace blockchain
{
class Network
{
public:
    virtual auto AddPeer(const p2p::Address& address) const noexcept
        -> bool = 0;
    virtual auto GetBalance() const noexcept -> Balance = 0;
    virtual auto GetConfirmations(const std::string& txid) const noexcept
        -> ChainHeight = 0;
    virtual auto GetHeight() const noexcept -> ChainHeight = 0;
    virtual auto GetPeerCount() const noexcept -> std::size_t = 0;
    virtual auto GetType() const noexcept -> Type = 0;
    virtual auto SendToAddress(
        const std::string& address,
        const Amount amount,
        const api::client::blockchain::BalanceTree& source) const noexcept
        -> std::string = 0;
    virtual auto SendToPaymentCode(
        const std::string& address,
        const Amount amount,
        const api::client::blockchain::PaymentCode& source) const noexcept
        -> std::string = 0;

    virtual auto Connect() noexcept -> bool = 0;
    virtual auto Disconnect() noexcept -> bool = 0;

    virtual ~Network() = default;

protected:
    Network() noexcept = default;

private:
    Network(const Network&) = delete;
    Network(Network&&) = delete;
    Network& operator=(const Network&) = delete;
    Network& operator=(Network&&) = delete;
};
}  // namespace blockchain
}  // namespace opentxs
#endif  // OPENTXS_BLOCKCHAIN_NETWORK_HPP
