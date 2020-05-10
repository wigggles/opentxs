// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Cfcheckpt.cpp"

#pragma once

#include <iosfwd>
#include <memory>
#include <vector>

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
class Cfcheckpt final : public internal::Cfcheckpt
{
public:
    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return payload_.at(position);
    }
    auto begin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto end() const noexcept -> const_iterator final
    {
        return const_iterator(this, payload_.size());
    }
    auto size() const noexcept -> std::size_t final { return payload_.size(); }
    auto Stop() const noexcept -> const value_type& final { return stop_; }
    auto Type() const noexcept -> filter::Type final { return type_; }

    ~Cfcheckpt() final = default;

private:
    friend opentxs::Factory;

    using BitcoinFormat = FilterPrefixBasic;

    const filter::Type type_;
    const filter::pHash stop_;
    const std::vector<filter::pHash> payload_;

    auto payload() const noexcept -> OTData final;

    Cfcheckpt(
        const api::internal::Core& api,
        const blockchain::Type network,
        const filter::Type type,
        const filter::Hash& stop,
        const std::vector<filter::pHash>& headers) noexcept;
    Cfcheckpt(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const filter::Type type,
        const filter::Hash& stop,
        const std::vector<filter::pHash>& headers) noexcept;
    Cfcheckpt(const Cfcheckpt&) = delete;
    Cfcheckpt(Cfcheckpt&&) = delete;
    auto operator=(const Cfcheckpt&) -> Cfcheckpt& = delete;
    auto operator=(Cfcheckpt &&) -> Cfcheckpt& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
