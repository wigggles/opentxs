// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Ping.cpp"

#pragma once

#include <memory>

#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
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
class Ping final : public internal::Ping
{
public:
    auto Nonce() const noexcept -> bitcoin::Nonce final { return nonce_; }

    ~Ping() final = default;

    auto payload() const noexcept -> OTData final;

private:
    friend opentxs::Factory;

    struct BitcoinFormat_60001 {
        NonceField nonce_{};

        BitcoinFormat_60001(const bitcoin::Nonce nonce) noexcept;
        BitcoinFormat_60001() noexcept;
    };

    const bitcoin::Nonce nonce_{};

    Ping(
        const api::internal::Core& api,
        const blockchain::Type network,
        const bitcoin::Nonce nonce) noexcept;
    Ping(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const bitcoin::Nonce nonce) noexcept;
    Ping(const Ping&) = delete;
    Ping(Ping&&) = delete;
    auto operator=(const Ping&) -> Ping& = delete;
    auto operator=(Ping &&) -> Ping& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
