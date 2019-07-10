// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/Proto.tpp"

#include "blockchain/client/Network.hpp"
#include "internal/blockchain/block/Block.hpp"

#include "Network.hpp"

// #define OT_METHOD
// "opentxs::blockchain::client::bitcoin::implementation::Network::"

namespace opentxs
{
blockchain::client::internal::Network* Factory::BlockchainNetworkBitcoin(
    const api::internal::Core& api,
    const blockchain::Type type,
    const std::string& seednode)
{
    using ReturnType = blockchain::client::bitcoin::implementation::Network;

    return new ReturnType{api, type, seednode};
}
}  // namespace opentxs

namespace opentxs::blockchain::client::bitcoin::implementation
{
Network::Network(
    const api::internal::Core& api,
    const Type type,
    const std::string& seednode)
    : ot_super(api, type, seednode)
{
    init();
}

std::unique_ptr<block::Header> Network::instantiate_header(
    const network::zeromq::Frame& payload) const noexcept
{
    using Type = block::Header::SerializedType;

    return std::unique_ptr<block::Header>{opentxs::Factory::BitcoinBlockHeader(
        api_, proto::Factory<Type>(payload))};
}

Network::~Network() { Shutdown(); }
}  // namespace opentxs::blockchain::client::bitcoin::implementation
