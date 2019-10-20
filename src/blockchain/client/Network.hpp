// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/network/zeromq/Pipeline.hpp"

#include "core/StateMachine.hpp"
#include "internal/blockchain/Blockchain.hpp"

#include <atomic>

namespace opentxs::blockchain::client::implementation
{
class Network : virtual public internal::Network,
                public opentxs::internal::StateMachine
{
public:
    bool AddPeer(const p2p::Address& address) const noexcept final
    {
        return peer_.AddPeer(address);
    }
    const blockchain::internal::Database& Database() const noexcept final
    {
        return database_;
    }
    ChainHeight GetConfirmations(const std::string& txid) const noexcept final;
    ChainHeight GetHeight() const noexcept final
    {
        return local_chain_height_.load();
    }
    std::size_t GetPeerCount() const noexcept final;
    Type GetType() const noexcept final { return chain_; }
    const network::zeromq::Pipeline& HeaderPipeline() const noexcept final
    {
        return new_headers_;
    }
    bool IsSynchronized() const noexcept final
    {
        return local_chain_height_.load() >= remote_chain_height_.load();
    }
    std::string SendToAddress(
        const std::string& address,
        const Amount amount,
        const api::client::blockchain::BalanceTree& source) const
        noexcept final;
    std::string SendToPaymentCode(
        const std::string& address,
        const Amount amount,
        const api::client::blockchain::PaymentCode& source) const
        noexcept final;
    void UpdateHeight(const block::Height height) const noexcept final;
    void UpdateLocalHeight(const block::Position position) const noexcept final;

    bool Connect() noexcept final;
    bool Disconnect() noexcept final;
    bool Shutdown() noexcept final;

    ~Network() override;

private:
    std::unique_ptr<blockchain::internal::Database> database_p_;
    std::unique_ptr<internal::FilterOracle> filter_p_;
    std::unique_ptr<internal::HeaderOracle> header_p_;
    std::unique_ptr<internal::PeerManager> peer_p_;
    std::unique_ptr<internal::Wallet> wallet_p_;

protected:
    const api::internal::Core& api_;
    const Type chain_;
    blockchain::internal::Database& database_;
    internal::HeaderOracle& header_;
    internal::PeerManager& peer_;

    // NOTE call init in every final constructor body
    void init() noexcept;
    void cleanup() noexcept;

    Network(
        const api::internal::Core& api,
        const Type type,
        const std::string& seednode) noexcept;

private:
    mutable std::atomic<block::Height> local_chain_height_;
    mutable std::atomic<block::Height> remote_chain_height_;
    OTZMQPipeline new_headers_;

    virtual std::unique_ptr<block::Header> instantiate_header(
        const network::zeromq::Frame& payload) const noexcept = 0;

    void process_header(network::zeromq::Message& in) noexcept;
    bool state_machine() noexcept;

    Network() = delete;
    Network(const Network&) = delete;
    Network(Network&&) = delete;
    Network& operator=(const Network&) = delete;
    Network& operator=(Network&&) = delete;
};
}  // namespace opentxs::blockchain::client::implementation
