// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/client/bitcoin/Network.hpp"  // IWYU pragma: associated

#include "Factory.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/blockchain/block/Header.hpp"

// #define OT_METHOD
// "opentxs::blockchain::client::bitcoin::implementation::Network::"

namespace opentxs::factory
{
auto BlockchainNetworkBitcoin(
    const api::internal::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const blockchain::Type type,
    const std::string& seednode,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::client::internal::Network>
{
    using ReturnType = blockchain::client::bitcoin::implementation::Network;

    return std::make_unique<ReturnType>(
        api, blockchain, type, seednode, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::client::bitcoin::implementation
{
Network::Network(
    const api::internal::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const Type type,
    const std::string& seednode,
    const std::string& shutdown)
    : ot_super(api, blockchain, type, seednode, shutdown)
{
    init();
}

auto Network::instantiate_header(const ReadView payload) const noexcept
    -> std::unique_ptr<block::Header>
{
    using Type = block::Header::SerializedType;

    return std::unique_ptr<block::Header>{opentxs::Factory::BitcoinBlockHeader(
        api_, proto::Factory<Type>(payload))};
}

Network::~Network() { Shutdown(); }
}  // namespace opentxs::blockchain::client::bitcoin::implementation
