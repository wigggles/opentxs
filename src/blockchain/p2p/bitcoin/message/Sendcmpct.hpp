// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"

#include <boost/endian/buffers.hpp>

#include <set>

namespace be = boost::endian;

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Sendcmpct final : virtual public bitcoin::Message
{
public:
    bool announce() const noexcept { return announce_; }
    std::uint64_t version() const noexcept { return version_; }

    ~Sendcmpct() final = default;

    OTData payload() const noexcept final;

private:
    struct Raw {
        be::little_int8_buf_t announce_;
        be::little_uint64_buf_t version_;

        Raw(bool announce, std::uint64_t version) noexcept;
        Raw() noexcept;
    };

    friend opentxs::Factory;

    bool announce_{};
    std::uint64_t version_{};

    Sendcmpct(
        const api::internal::Core& api,
        const blockchain::Type network,
        const bool announce,
        const std::uint64_t version) noexcept;
    Sendcmpct(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const bool announce,
        const std::uint64_t version) noexcept(false);
    Sendcmpct(const Sendcmpct&) = delete;
    Sendcmpct(Sendcmpct&&) = delete;
    Sendcmpct& operator=(const Sendcmpct&) = delete;
    Sendcmpct& operator=(Sendcmpct&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
