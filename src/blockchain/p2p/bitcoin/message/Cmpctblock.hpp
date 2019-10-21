// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"

#include <set>

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Cmpctblock final : virtual public bitcoin::Message
{
public:
    OTData getRawCmpctblock() const noexcept
    {
        return Data::Factory(raw_cmpctblock_);
    }

    ~Cmpctblock() final = default;

    OTData payload() const noexcept final;

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
    Cmpctblock& operator=(const Cmpctblock&) = delete;
    Cmpctblock& operator=(Cmpctblock&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
