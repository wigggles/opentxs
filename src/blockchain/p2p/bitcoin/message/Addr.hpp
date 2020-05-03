// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Addr.cpp"

#pragma once

#include <iosfwd>
#include <memory>
#include <utility>
#include <vector>

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

namespace internal
{
struct Address;
}  // namespace internal
}  // namespace p2p
}  // namespace blockchain

class Factory;
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Addr final : public internal::Addr
{
public:
    using pAddress = std::unique_ptr<blockchain::p2p::internal::Address>;
    using AddressVector = std::vector<pAddress>;

    static std::pair<p2p::Network, OTData> ExtractAddress(
        AddressByteField in) noexcept;
    static bool SerializeTimestamp(const ProtocolVersion version) noexcept;

    const value_type& at(const std::size_t position) const noexcept(false) final
    {
        return *payload_.at(position);
    }
    const_iterator begin() const noexcept final
    {
        return const_iterator(this, 0);
    }
    const_iterator end() const noexcept final
    {
        return const_iterator(this, payload_.size());
    }
    bool SerializeTimestamp() const noexcept
    {
        return SerializeTimestamp(version_);
    }
    std::size_t size() const noexcept final { return payload_.size(); }

    ~Addr() final = default;

private:
    friend opentxs::Factory;

    struct BitcoinFormat_31402 {
        TimestampField32 time_;
        AddressVersion data_;

        BitcoinFormat_31402(
            const blockchain::Type chain,
            const ProtocolVersion version,
            const p2p::internal::Address& address);
        BitcoinFormat_31402();
    };

    const ProtocolVersion version_;
    const AddressVector payload_;

    OTData payload() const noexcept final;

    Addr(
        const api::internal::Core& api,
        const blockchain::Type network,
        const ProtocolVersion version,
        AddressVector&& addresses) noexcept;
    Addr(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const ProtocolVersion version,
        AddressVector&& addresses) noexcept;
    Addr(const Addr&) = delete;
    Addr(Addr&&) = delete;
    Addr& operator=(const Addr&) = delete;
    Addr& operator=(Addr&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
