// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <tuple>

#include "1_Internal.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BloomFilter.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

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

namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message
{
using ClientFilterTypeField = be::little_uint8_buf_t;
using HashField = std::array<std::byte, 32>;
using InventoryTypeField = be::little_uint32_buf_t;

struct FilterPrefixBasic {
    ClientFilterTypeField type_;
    HashField hash_;

    auto Hash() const noexcept -> filter::pHash;
    auto Type(const blockchain::Type chain) const noexcept -> filter::Type;

    FilterPrefixBasic(
        const blockchain::Type chain,
        const filter::Type type,
        const filter::Hash& hash) noexcept(false);
    FilterPrefixBasic() noexcept;
};
struct FilterPrefixChained {
    ClientFilterTypeField type_;
    HashField hash_;
    HashField previous_;

    auto Previous() const noexcept -> filter::pHash;
    auto Stop() const noexcept -> filter::pHash;
    auto Type(const blockchain::Type chain) const noexcept -> filter::Type;

    FilterPrefixChained(
        const blockchain::Type chain,
        const filter::Type type,
        const filter::Hash& stop,
        const filter::Hash& prefix) noexcept(false);
    FilterPrefixChained() noexcept;
};
struct FilterRequest {
    ClientFilterTypeField type_;
    HeightField start_;
    HashField stop_;

    auto Start() const noexcept -> block::Height;
    auto Stop() const noexcept -> filter::pHash;
    auto Type(const blockchain::Type chain) const noexcept -> filter::Type;

    FilterRequest(
        const blockchain::Type chain,
        const filter::Type type,
        const block::Height start,
        const filter::Hash& stop) noexcept(false);
    FilterRequest() noexcept;
};

auto VerifyChecksum(
    const api::internal::Core& api,
    const Header& header,
    const network::zeromq::Frame& payload) noexcept -> bool;
}  // namespace opentxs::blockchain::p2p::bitcoin::message

namespace opentxs::blockchain::p2p::bitcoin::message::internal
{
struct Addr : virtual public bitcoin::Message {
    using value_type = p2p::internal::Address;
    using const_iterator =
        iterator::Bidirectional<const Addr, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
};
struct Block : virtual public bitcoin::Message {
    virtual auto GetBlock() const noexcept -> OTData = 0;
};
struct Blocktxn : virtual public bitcoin::Message {
    virtual auto BlockTransactions() const noexcept -> OTData = 0;
};
struct Cfcheckpt : virtual public bitcoin::Message {
    using value_type = filter::Hash;
    using const_iterator =
        iterator::Bidirectional<const Cfcheckpt, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto Stop() const noexcept -> const value_type& = 0;
    virtual auto Type() const noexcept -> filter::Type = 0;
};
struct Cfheaders : virtual public bitcoin::Message {
    using value_type = filter::Hash;
    using const_iterator =
        iterator::Bidirectional<const Cfheaders, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto Previous() const noexcept -> const value_type& = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto Stop() const noexcept -> const value_type& = 0;
    virtual auto Type() const noexcept -> filter::Type = 0;
};
struct Cfilter : virtual public bitcoin::Message {
    virtual auto Bits() const noexcept -> std::uint8_t = 0;
    virtual auto ElementCount() const noexcept -> std::uint32_t = 0;
    virtual auto FPRate() const noexcept -> std::uint32_t = 0;
    virtual auto Filter() const noexcept -> ReadView = 0;
    virtual auto Hash() const noexcept -> const filter::Hash& = 0;
    virtual auto Type() const noexcept -> filter::Type = 0;
};
struct Filteradd : virtual public bitcoin::Message {
    virtual auto Element() const noexcept -> OTData = 0;
};
struct Filterclear : virtual public bitcoin::Message {
};
struct Filterload : virtual public bitcoin::Message {
    virtual auto Filter() const noexcept -> OTBloomFilter = 0;
};
struct Getaddr : virtual public bitcoin::Message {
};
struct Getcfcheckpt : virtual public bitcoin::Message {
    virtual auto Stop() const noexcept -> const filter::Hash& = 0;
    virtual auto Type() const noexcept -> filter::Type = 0;
};
struct Getcfheaders : virtual public bitcoin::Message {
    virtual auto Start() const noexcept -> block::Height = 0;
    virtual auto Stop() const noexcept -> const filter::Hash& = 0;
    virtual auto Type() const noexcept -> filter::Type = 0;
};
struct Getcfilters : virtual public bitcoin::Message {
    virtual auto Start() const noexcept -> block::Height = 0;
    virtual auto Stop() const noexcept -> const filter::Hash& = 0;
    virtual auto Type() const noexcept -> filter::Type = 0;
};
struct Getdata : virtual public bitcoin::Message {
    using value_type = blockchain::bitcoin::Inventory;
    using const_iterator =
        iterator::Bidirectional<const Getdata, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
};
struct Getheaders : virtual public bitcoin::Message {
    using value_type = block::Hash;
    using const_iterator =
        iterator::Bidirectional<const Getheaders, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto StopHash() const noexcept -> block::pHash = 0;
    virtual auto Version() const noexcept -> ProtocolVersionUnsigned = 0;
};
struct Headers : virtual public bitcoin::Message {
    using value_type = block::bitcoin::Header;
    using const_iterator =
        iterator::Bidirectional<const Headers, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
};
struct Inv : virtual public bitcoin::Message {
    using value_type = blockchain::bitcoin::Inventory;
    using const_iterator = iterator::Bidirectional<const Inv, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
};
struct Mempool : virtual public bitcoin::Message {
};
struct Notfound : virtual public bitcoin::Message {
    using value_type = blockchain::bitcoin::Inventory;
    using const_iterator =
        iterator::Bidirectional<const Notfound, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
};
struct Ping : virtual public bitcoin::Message {
    virtual auto Nonce() const noexcept -> bitcoin::Nonce = 0;
};
struct Pong : virtual public bitcoin::Message {
    virtual auto Nonce() const noexcept -> bitcoin::Nonce = 0;
};
struct Sendheaders : virtual public bitcoin::Message {
};
struct Verack : virtual public bitcoin::Message {
};
struct Version : virtual public bitcoin::Message {
    virtual auto Height() const noexcept -> block::Height = 0;
    virtual auto LocalAddress() const noexcept -> tcp::endpoint = 0;
    virtual auto LocalServices() const noexcept
        -> std::set<blockchain::p2p::Service> = 0;
    virtual auto Nonce() const noexcept -> api::client::blockchain::Nonce = 0;
    virtual auto ProtocolVersion() const noexcept
        -> bitcoin::ProtocolVersion = 0;
    virtual auto Relay() const noexcept -> bool = 0;
    virtual auto RemoteAddress() const noexcept -> tcp::endpoint = 0;
    virtual auto RemoteServices() const noexcept
        -> std::set<blockchain::p2p::Service> = 0;
    virtual auto UserAgent() const noexcept -> const std::string& = 0;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::internal
