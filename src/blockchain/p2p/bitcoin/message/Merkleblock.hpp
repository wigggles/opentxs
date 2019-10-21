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
class Merkleblock final : virtual public bitcoin::Message
{
public:
    OTData getBlockHeader() const noexcept
    {
        return Data::Factory(block_header_);
    }
    TxnCount getTxnCount() const noexcept { return txn_count_; }
    const std::vector<OTData>& getHashes() const noexcept { return hashes_; }
    const std::vector<std::byte>& getFlags() const noexcept { return flags_; }

    ~Merkleblock() final = default;

    OTData payload() const noexcept final;

private:
    friend opentxs::Factory;

    struct Raw {
        BlockHeaderField block_header_;
        TxnCountField txn_count_;

        Raw(const Data& block_header, const TxnCount txn_count) noexcept;
        Raw() noexcept;
    };

    const OTData block_header_;
    const TxnCount txn_count_{};
    const std::vector<OTData> hashes_;
    const std::vector<std::byte> flags_;

    Merkleblock(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& block_header,
        const TxnCount txn_count,
        const std::vector<OTData>& hashes,
        const std::vector<std::byte>& flags) noexcept;
    Merkleblock(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const Data& block_header,
        const TxnCount txn_count,
        const std::vector<OTData>& hashes,
        const std::vector<std::byte>& flags) noexcept(false);
    Merkleblock(const Merkleblock&) = delete;
    Merkleblock(Merkleblock&&) = delete;
    Merkleblock& operator=(const Merkleblock&) = delete;
    Merkleblock& operator=(Merkleblock&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
