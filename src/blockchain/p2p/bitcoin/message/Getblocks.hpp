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
class Getblocks final : virtual public bitcoin::Message
{
public:
    bitcoin::ProtocolVersionUnsigned version() const noexcept
    {
        return version_;
    }
    std::size_t hashCount() const noexcept { return header_hashes_.size(); }
    const std::vector<OTData>& getHashes() const noexcept
    {
        return header_hashes_;
    }
    OTData getStopHash() const noexcept { return Data::Factory(stop_hash_); }

    ~Getblocks() final = default;

    OTData payload() const noexcept final;

private:
    friend opentxs::Factory;

    struct Raw {
        ProtocolVersionField version_;
        std::vector<BlockHeaderHashField> header_hashes_;
        BlockHeaderHashField stop_hash_;

        Raw(ProtocolVersionUnsigned version,
            const std::vector<OTData>& header_hashes,
            const Data& stop_hash) noexcept;
        Raw() noexcept;
    };

    const bitcoin::ProtocolVersionUnsigned version_;
    const std::vector<OTData> header_hashes_;
    const OTData stop_hash_;

    Getblocks(
        const api::internal::Core& api,
        const blockchain::Type network,
        const bitcoin::ProtocolVersionUnsigned version,
        const std::vector<OTData>& header_hashes,
        const Data& stop_hash) noexcept;
    Getblocks(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const bitcoin::ProtocolVersionUnsigned version,
        const std::vector<OTData>& header_hashes,
        const Data& stop_hash) noexcept(false);
    Getblocks(const Getblocks&) = delete;
    Getblocks(Getblocks&&) = delete;
    Getblocks& operator=(const Getblocks&) = delete;
    Getblocks& operator=(Getblocks&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
