// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
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
}  // namespace opentxs

namespace ot = opentxs;

namespace opentxs::blockchain::p2p::bitcoin
{
class Message
{
public:
    static auto MaxPayload() -> std::size_t;

    auto Encode() const -> OTData;

    auto header() const -> const Header& { return *header_; }
    virtual auto payload() const noexcept -> OTData = 0;

    virtual ~Message() = default;

protected:
    const ot::api::client::Manager& api_;
    std::unique_ptr<Header> header_;

    void verify_checksum() const noexcept(false);

    void init_hash() noexcept;

    Message(
        const api::client::Manager& api,
        const blockchain::Type network,
        const bitcoin::Command command) noexcept;
    Message(
        const api::client::Manager& api,
        std::unique_ptr<Header> header) noexcept;

private:
    auto calculate_checksum(const Data& payload) const noexcept -> OTData;

    Message() = delete;
    Message(const Message&) = delete;
    Message(Message&&) = delete;
    auto operator=(const Message&) -> Message& = delete;
    auto operator=(Message &&) -> Message& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin
