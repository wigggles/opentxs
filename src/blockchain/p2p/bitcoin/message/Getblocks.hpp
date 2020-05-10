// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Getblocks.cpp"

#pragma once

#include <algorithm>
#include <array>
#include <cstring>
#include <iosfwd>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Pimpl.hpp"
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
class Getblocks final : virtual public bitcoin::Message
{
public:
    OPENTXS_EXPORT auto getHashes() const noexcept -> const std::vector<OTData>&
    {
        return header_hashes_;
    }
    OPENTXS_EXPORT auto getStopHash() const noexcept -> OTData
    {
        return Data::Factory(stop_hash_);
    }
    OPENTXS_EXPORT auto hashCount() const noexcept -> std::size_t
    {
        return header_hashes_.size();
    }
    OPENTXS_EXPORT auto payload() const noexcept -> OTData final
    {
        try {
            Raw raw_data(version_, header_hashes_, stop_hash_);

            auto output =
                Data::Factory(&raw_data.version_, sizeof(raw_data.version_));

            const auto size = CompactSize(header_hashes_.size()).Encode();
            output->Concatenate(size.data(), size.size());

            for (const auto& raw_hash : raw_data.header_hashes_) {
                output->Concatenate(raw_hash.data(), sizeof(raw_hash));
            }

            output->Concatenate(
                raw_data.stop_hash_.data(), sizeof(raw_data.stop_hash_));

            return output;
        } catch (...) {
            return Data::Factory();
        }
    }
    OPENTXS_EXPORT auto version() const noexcept
        -> bitcoin::ProtocolVersionUnsigned
    {
        return version_;
    }

    OPENTXS_EXPORT ~Getblocks() final = default;

private:
    friend opentxs::Factory;

    struct Raw {
        ProtocolVersionField version_;
        std::vector<BlockHeaderHashField> header_hashes_;
        BlockHeaderHashField stop_hash_;

        Raw(ProtocolVersionUnsigned version,
            const std::vector<OTData>& header_hashes,
            const Data& stop_hash) noexcept(false)
            : version_(version)
            , header_hashes_()
            , stop_hash_()
        {
            if (stop_hash.size() != sizeof(stop_hash_)) {
                throw std::runtime_error("Invalid stop hash");
            }

            std::memcpy(stop_hash_.data(), stop_hash.data(), stop_hash.size());

            for (const auto& hash : header_hashes) {
                BlockHeaderHashField tempHash;

                if (hash->size() != sizeof(tempHash)) {
                    throw std::runtime_error("Invalid hash");
                }

                std::memcpy(tempHash.data(), hash->data(), hash->size());
                header_hashes_.push_back(tempHash);
            }
        }
        Raw() noexcept
            : version_(2)  // TODO
            , header_hashes_()
            , stop_hash_()
        {
        }
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
    auto operator=(const Getblocks&) -> Getblocks& = delete;
    auto operator=(Getblocks &&) -> Getblocks& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
