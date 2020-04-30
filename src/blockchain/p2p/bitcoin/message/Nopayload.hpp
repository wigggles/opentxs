// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Nopayload.cpp"

#pragma once

#include <memory>
#include <utility>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain

class Factory;
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
template <typename InterfaceType>
class Nopayload final : virtual public InterfaceType
{
public:
    ~Nopayload() final = default;

private:
    friend opentxs::Factory;

    OTData payload() const noexcept final { return Data::Factory(); }

    Nopayload(
        const api::internal::Core& api,
        const blockchain::Type network,
        const bitcoin::Command command) noexcept
        : Message(api, network, command)
    {
        Message::init_hash();
    }
    Nopayload(
        const api::internal::Core& api,
        std::unique_ptr<Header> header) noexcept
        : Message(api, std::move(header))
    {
    }
    Nopayload(const Nopayload&) = delete;
    Nopayload(Nopayload&&) = delete;
    Nopayload& operator=(const Nopayload&) = delete;
    Nopayload& operator=(Nopayload&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
