// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Cfheaders.cpp"

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
class Cfheaders final : public internal::Cfheaders
{
public:
    const value_type& at(const std::size_t position) const noexcept(false) final
    {
        return payload_.at(position);
    }
    const_iterator begin() const noexcept final
    {
        return const_iterator(this, 0);
    }
    const_iterator end() const noexcept final
    {
        return const_iterator(this, payload_.size());
    }
    const value_type& Previous() const noexcept final { return previous_; }
    std::size_t size() const noexcept final { return payload_.size(); }
    const value_type& Stop() const noexcept final { return stop_; }
    filter::Type Type() const noexcept final { return type_; }

    ~Cfheaders() final = default;

private:
    friend opentxs::Factory;

    using BitcoinFormat = FilterPrefixChained;

    const filter::Type type_;
    const filter::pHash stop_;
    const filter::pHash previous_;
    const std::vector<filter::pHash> payload_;

    OTData payload() const noexcept final;

    Cfheaders(
        const api::internal::Core& api,
        const blockchain::Type network,
        const filter::Type type,
        const filter::Hash& stop,
        const filter::Hash& previous,
        const std::vector<filter::pHash>& headers) noexcept;
    Cfheaders(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const filter::Type type,
        const filter::Hash& stop,
        const filter::Hash& previous,
        const std::vector<filter::pHash>& headers) noexcept;
    Cfheaders(const Cfheaders&) = delete;
    Cfheaders(Cfheaders&&) = delete;
    Cfheaders& operator=(const Cfheaders&) = delete;
    Cfheaders& operator=(Cfheaders&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
