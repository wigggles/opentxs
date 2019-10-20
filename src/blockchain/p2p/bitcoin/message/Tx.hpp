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
class Tx final : virtual public bitcoin::Message
{
public:
    OTData getRawTx() const noexcept { return Data::Factory(raw_tx_); }

    ~Tx() final = default;

    OTData payload() const noexcept final;

private:
    friend opentxs::Factory;

    const OTData raw_tx_;

    Tx(const api::internal::Core& api,
       const blockchain::Type network,
       const Data& raw_tx) noexcept;
    Tx(const api::internal::Core& api,
       std::unique_ptr<Header> header,
       const Data& raw_tx) noexcept(false);
    Tx(const Tx&) = delete;
    Tx(Tx&&) = delete;
    Tx& operator=(const Tx&) = delete;
    Tx& operator=(Tx&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
