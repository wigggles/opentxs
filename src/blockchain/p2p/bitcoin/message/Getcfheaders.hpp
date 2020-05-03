// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Getcfheaders.cpp"

#pragma once

#include <memory>

#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
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
class Getcfheaders final : public internal::Getcfheaders
{
public:
    block::Height Start() const noexcept final { return start_; }
    const filter::Hash& Stop() const noexcept final { return stop_; }
    filter::Type Type() const noexcept final { return type_; }

    ~Getcfheaders() final = default;

private:
    friend opentxs::Factory;

    using BitcoinFormat = FilterRequest;

    const filter::Type type_;
    const block::Height start_;
    const filter::pHash stop_;

    OTData payload() const noexcept final;

    Getcfheaders(
        const api::internal::Core& api,
        const blockchain::Type network,
        const filter::Type type,
        const block::Height start,
        const filter::Hash& stop) noexcept;
    Getcfheaders(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const filter::Type type,
        const block::Height start,
        const filter::Hash& stop) noexcept;
    Getcfheaders(const Getcfheaders&) = delete;
    Getcfheaders(Getcfheaders&&) = delete;
    Getcfheaders& operator=(const Getcfheaders&) = delete;
    Getcfheaders& operator=(Getcfheaders&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
