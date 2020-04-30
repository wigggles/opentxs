// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Pong.cpp"

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
class Pong final : public internal::Pong
{
public:
    bitcoin::Nonce Nonce() const noexcept final { return nonce_; }

    ~Pong() final = default;

private:
    friend opentxs::Factory;

    struct BitcoinFormat_60001 {
        NonceField nonce_{};

        BitcoinFormat_60001(const bitcoin::Nonce nonce) noexcept;
        BitcoinFormat_60001() noexcept;
    };

    const bitcoin::Nonce nonce_{};

    OTData payload() const noexcept final;

    Pong(
        const api::internal::Core& api,
        const blockchain::Type network,
        const bitcoin::Nonce nonce) noexcept;
    Pong(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const bitcoin::Nonce nonce) noexcept;
    Pong(const Pong&) = delete;
    Pong(Pong&&) = delete;
    Pong& operator=(const Pong&) = delete;
    Pong& operator=(Pong&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
