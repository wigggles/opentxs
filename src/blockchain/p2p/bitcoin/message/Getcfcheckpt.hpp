// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Getcfcheckpt.cpp"

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
class Getcfcheckpt final : public internal::Getcfcheckpt
{
public:
    auto Stop() const noexcept -> const filter::Hash& final { return stop_; }
    auto Type() const noexcept -> filter::Type final { return type_; }

    ~Getcfcheckpt() final = default;

private:
    friend opentxs::Factory;

    using BitcoinFormat = FilterPrefixBasic;

    const filter::Type type_;
    const filter::pHash stop_;

    auto payload() const noexcept -> OTData final;

    Getcfcheckpt(
        const api::internal::Core& api,
        const blockchain::Type network,
        const filter::Type type,
        const filter::Hash& stop) noexcept;
    Getcfcheckpt(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const filter::Type type,
        const filter::Hash& stop) noexcept;
    Getcfcheckpt(const Getcfcheckpt&) = delete;
    Getcfcheckpt(Getcfcheckpt&&) = delete;
    auto operator=(const Getcfcheckpt&) -> Getcfcheckpt& = delete;
    auto operator=(Getcfcheckpt &&) -> Getcfcheckpt& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
