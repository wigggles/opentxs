// Copyright (c) 2010-2020 The Open-Transactions developers
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
class Getblocktxn final : virtual public bitcoin::Message
{
public:
    OTData getBlockHash() const noexcept { return Data::Factory(block_hash_); }
    const std::vector<std::size_t>& getIndices() const noexcept
    {
        return txn_indices_;
    }

    ~Getblocktxn() final = default;

    OTData payload() const noexcept final;

private:
    friend opentxs::Factory;

    const OTData block_hash_;
    const std::vector<std::size_t> txn_indices_;

    Getblocktxn(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& block_hash,
        const std::vector<std::size_t>& txn_indices) noexcept;
    Getblocktxn(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const Data& block_hash,
        const std::vector<std::size_t>& txn_indices) noexcept(false);
    Getblocktxn(const Getblocktxn&) = delete;
    Getblocktxn(Getblocktxn&&) = delete;
    Getblocktxn& operator=(const Getblocktxn&) = delete;
    Getblocktxn& operator=(Getblocktxn&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
