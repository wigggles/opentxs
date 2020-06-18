// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Filterload.cpp"

#pragma once

#include <memory>

#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BloomFilter.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
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
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Filterload final : virtual public internal::Filterload
{
public:
    virtual auto Filter() const noexcept -> OTBloomFilter { return payload_; }

    Filterload(
        const api::client::Manager& api,
        const blockchain::Type network,
        const blockchain::BloomFilter& filter) noexcept;
    Filterload(
        const api::client::Manager& api,
        std::unique_ptr<Header> header,
        const blockchain::BloomFilter& filter) noexcept;

    ~Filterload() final = default;

private:
    const OTBloomFilter payload_;

    auto payload() const noexcept -> OTData final
    {
        return payload_->Serialize();
    }

    Filterload(const Filterload&) = delete;
    Filterload(Filterload&&) = delete;
    auto operator=(const Filterload&) -> Filterload& = delete;
    auto operator=(Filterload &&) -> Filterload& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
