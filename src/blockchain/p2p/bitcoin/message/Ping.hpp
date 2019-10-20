// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Ping final : public internal::Ping
{
public:
    bitcoin::Nonce Nonce() const noexcept final { return nonce_; }

    ~Ping() final = default;

    OTData payload() const noexcept final;

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
    Ping& operator=(const Ping&) = delete;
    Ping& operator=(Ping&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
