// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Cmpctblock.cpp"

#pragma once

#include <memory>
#include <set>

#include "blockchain/p2p/bitcoin/Message.hpp"
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

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Cmpctblock final : virtual public bitcoin::Message
{
public:
    auto getRawCmpctblock() const noexcept -> OTData
    {
        return Data::Factory(raw_cmpctblock_);
    }

    ~Cmpctblock() final = default;

    auto payload() const noexcept -> OTData final;

private:
    friend opentxs::Factory;

    const OTData raw_cmpctblock_;

    Cmpctblock(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& raw_cmpctblock) noexcept;
    Cmpctblock(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const Data& raw_cmpctblock) noexcept(false);
    Cmpctblock(const Cmpctblock&) = delete;
    Cmpctblock(Cmpctblock&&) = delete;
    auto operator=(const Cmpctblock&) -> Cmpctblock& = delete;
    auto operator=(Cmpctblock &&) -> Cmpctblock& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
