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

#include "internal/api/Api.hpp"

#include "Network.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::Network::"

namespace opentxs::blockchain::client::implementation
{
Network::Network(
    const api::internal::Core& api,
    const Type type,
    const std::string& seednode) noexcept
    : StateMachine(std::bind(&Network::state_machine, this))
    , database_p_(Factory::BlockchainDatabase(api, *this, type))
    , filter_p_()
    , header_p_(Factory::HeaderOracle(api, *this, type))
    , peer_p_(Factory::BlockchainPeerManager(api, *this, type, seednode))
    , wallet_p_()
    , api_(api)
    , chain_(type)
    , database_(*database_p_)
    , header_(*header_p_)
    , peer_(*peer_p_)
    , local_chain_height_(0)
    , remote_chain_height_(0)
    , new_headers_(
          Factory::Pipeline(api_, api_.ZeroMQ(), [this](auto& in) -> void {
              this->process_header(in);
          }))
{
    OT_ASSERT(database_p_);
    OT_ASSERT(header_p_);
    OT_ASSERT(peer_p_);
}

bool Network::Connect() noexcept
{
    // TODO

    return false;
}

void Network::cleanup() noexcept { new_headers_->Close(); }

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

std::size_t Network::GetPeerCount() const noexcept
{
    // TODO

    return {};
}

void Network::init() noexcept
{
    local_chain_height_.store(header_.BestChain().first);
    peer_.init();
}

void Network::process_header(network::zeromq::Message& in) noexcept
{
    const auto& body = in.Body();

    if (1 != body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        return;
    }

    const auto& payload = body.at(0);
    header_.AddHeader(instantiate_header(payload));
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
    // TODO

    return false;
}

bool Network::state_machine() noexcept
{
    // TODO

    return false;
}

void Network::UpdateHeight(const block::Height height) const noexcept
{
    remote_chain_height_.store(std::max(height, remote_chain_height_.load()));
    Trigger();
}

void Network::UpdateLocalHeight(const block::Position position) const noexcept
{
    const auto& [height, hash] = position;
    LogNormal(blockchain::internal::DisplayString(chain_))(
        " chain updated to hash ")(hash->asHex())(" at height ")(height)
        .Flush();
    local_chain_height_.store(height);
    Trigger();
}

Network::~Network() { Stop().get(); }
}  // namespace opentxs::blockchain::client::implementation
