// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "internal/api/client/Client.hpp"
#include "internal/api/Api.hpp"

#include "Network.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::Network::"

namespace opentxs::blockchain::client::implementation
{
Network::Network(
    const api::internal::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const Type type,
    const std::string& seednode) noexcept
    : StateMachine(std::bind(&Network::state_machine, this))
    , database_p_(Factory::BlockchainDatabase(
          api,
          *this,
          blockchain.BlockchainDB(),
          type))
    , filter_p_(Factory::BlockchainFilterOracle(api, *this, *database_p_))
    , header_p_(Factory::HeaderOracle(api, *this, *database_p_, type))
    , peer_p_(Factory::BlockchainPeerManager(
          api,
          *this,
          *database_p_,
          type,
          seednode))
    , wallet_p_()
    , api_(api)
    , chain_(type)
    , database_(*database_p_)
    , filters_(*filter_p_)
    , header_(*header_p_)
    , peer_(*peer_p_)
    , running_(Flag::Factory(true))
    , local_chain_height_(0)
    , remote_chain_height_(0)
    , processing_headers_(Flag::Factory(false))
    , new_headers_(Factory::Pipeline(
          api_,
          api_.ZeroMQ(),
          [this](auto& in) { this->process_header(in); }))
    , new_filters_(Factory::Pipeline(api_, api_.ZeroMQ(), [this](auto& in) {
        this->process_filter(in);
    }))
{
    OT_ASSERT(database_p_);
    OT_ASSERT(filter_p_);
    OT_ASSERT(header_p_);
    OT_ASSERT(peer_p_);

    filters_.Start();
    api_.Schedule(std::chrono::seconds(30), [this]() { Trigger(); });
}

auto Network::AddPeer(const p2p::Address& address) const noexcept -> bool
{
    if (false == running_.get()) { return false; }

    return peer_.AddPeer(address);
}

auto Network::Connect() noexcept -> bool
{
    if (false == running_.get()) { return false; }

    return peer_.Connect();
}

bool Network::Disconnect() noexcept
{
    // TODO

    return false;
}

ChainHeight Network::GetConfirmations(const std::string& txid) const noexcept
{
    // TODO

    return -1;
}

auto Network::GetPeerCount() const noexcept -> std::size_t
{
    if (false == running_.get()) { return false; }

    return peer_.GetPeerCount();
}

void Network::init() noexcept
{
    local_chain_height_.store(header_.BestChain().first);
}

void Network::process_filter(network::zeromq::Message& in) noexcept
{
    if (false == running_.get()) { return; }

    const auto& body = in.Body();

    if (3 != body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        return;
    }

    filters_.AddFilter(
        body.at(0).as<filter::Type>(),
        Data::Factory(body.at(1)),
        Data::Factory(body.at(2)));
}

void Network::process_header(network::zeromq::Message& in) noexcept
{
    struct Cleanup {
        Cleanup(Flag& flag)
            : flag_(flag)
        {
            flag_.On();
        }

        ~Cleanup() { flag_.Off(); }

    private:
        Flag& flag_;
    };

    if (false == running_.get()) { return; }

    auto cleanup = Cleanup(processing_headers_);
    const auto& header = in.Header();
    const auto& body = in.Body();

    if (1 > header.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid header").Flush();

        return;
    }

    if (1 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        return;
    }

    using Promise = std::promise<void>;
    auto pPromise = std::unique_ptr<Promise>{
        reinterpret_cast<Promise*>(header.at(0).as<std::uintptr_t>())};

    OT_ASSERT(pPromise);

    auto& promise = *pPromise;
    auto headers = std::vector<std::unique_ptr<block::Header>>{};

    for (const auto& header : body) {
        headers.emplace_back(instantiate_header(header));
    }

    header_.AddHeaders(headers);
    processing_headers_->Off();
    promise.set_value();
    Trigger();
}

void Network::RequestFilters(
    const filter::Type type,
    const block::Height start,
    const block::Hash& stop) const noexcept
{
    if (false == running_.get()) { return; }

    peer_.RequestFilters(type, start, stop);
}

std::string Network::SendToAddress(
    const std::string& address,
    const Amount amount,
    const api::client::blockchain::BalanceTree& source) const noexcept
{
    // TODO

    return {};
}

std::string Network::SendToPaymentCode(
    const std::string& address,
    const Amount amount,
    const api::client::blockchain::PaymentCode& source) const noexcept
{
    // TODO

    return {};
}

bool Network::Shutdown() noexcept
{
    running_->Off();
    new_filters_->Close();
    new_headers_->Close();
    Stop().get();
    peer_.Shutdown();
    filters_.Shutdown();

    return true;
}

bool Network::state_machine() noexcept
{
    filters_.CheckBlocks();

    if (false == IsSynchronized()) {
        if (false == processing_headers_.get()) { peer_.RequestHeaders(); }

        Sleep(std::chrono::milliseconds(20));

        return true;
    }

    return false;
}

void Network::UpdateHeight(const block::Height height) const noexcept
{
    if (false == running_.get()) { return; }

    remote_chain_height_.store(std::max(height, remote_chain_height_.load()));
    Trigger();
}

void Network::UpdateLocalHeight(const block::Position position) const noexcept
{
    if (false == running_.get()) { return; }

    const auto& [height, hash] = position;
    LogNormal(blockchain::internal::DisplayString(chain_))(
        " chain updated to hash ")(hash->asHex())(" at height ")(height)
        .Flush();
    local_chain_height_.store(height);
    Trigger();
}

Network::~Network() { Shutdown(); }
}  // namespace opentxs::blockchain::client::implementation
