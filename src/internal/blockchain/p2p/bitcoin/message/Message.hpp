// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/blockchain/BloomFilter.hpp"
#include "opentxs/iterator/Bidirectional.hpp"
#include "opentxs/Bytes.hpp"

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"

#include <array>
#include <set>
#include <tuple>

namespace opentxs::blockchain::p2p::bitcoin::message
{
using ClientFilterTypeField = be::little_uint8_buf_t;
using HashField = std::array<std::byte, 32>;
using InventoryTypeField = be::little_uint32_buf_t;

struct FilterPrefixBasic {
    ClientFilterTypeField type_;
    HashField hash_;

    filter::pHash Hash() const noexcept;
    filter::Type Type(const blockchain::Type chain) const noexcept;

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

    filter::pHash Previous() const noexcept;
    filter::pHash Stop() const noexcept;
    filter::Type Type(const blockchain::Type chain) const noexcept;

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

    block::Height Start() const noexcept;
    filter::pHash Stop() const noexcept;
    filter::Type Type(const blockchain::Type chain) const noexcept;

    FilterRequest(
        const blockchain::Type chain,
        const filter::Type type,
        const block::Height start,
        const filter::Hash& stop) noexcept(false);
    FilterRequest() noexcept;
};

bool VerifyChecksum(
    const api::internal::Core& api,
    const Header& header,
    const network::zeromq::Frame& payload) noexcept;
}  // namespace opentxs::blockchain::p2p::bitcoin::message

namespace opentxs::blockchain::p2p::bitcoin::message::internal
{
struct Addr : virtual public bitcoin::Message {
    using value_type = p2p::internal::Address;
    using const_iterator =
        iterator::Bidirectional<const Addr, const value_type>;

    virtual const value_type& at(const std::size_t position) const
        noexcept(false) = 0;
    virtual const_iterator begin() const noexcept = 0;
    virtual const_iterator end() const noexcept = 0;
    virtual std::size_t size() const noexcept = 0;
};
struct Block : virtual public bitcoin::Message {
    virtual OTData GetBlock() const noexcept = 0;
};
struct Blocktxn : virtual public bitcoin::Message {
    virtual OTData BlockTransactions() const noexcept = 0;
};
struct Cfcheckpt : virtual public bitcoin::Message {
    using value_type = filter::Hash;
    using const_iterator =
        iterator::Bidirectional<const Cfcheckpt, const value_type>;

    virtual const value_type& at(const std::size_t position) const
        noexcept(false) = 0;
    virtual const_iterator begin() const noexcept = 0;
    virtual const_iterator end() const noexcept = 0;
    virtual std::size_t size() const noexcept = 0;
    virtual const value_type& Stop() const noexcept = 0;
    virtual filter::Type Type() const noexcept = 0;
};
struct Cfheaders : virtual public bitcoin::Message {
    using value_type = filter::Hash;
    using const_iterator =
        iterator::Bidirectional<const Cfheaders, const value_type>;

    virtual const value_type& at(const std::size_t position) const
        noexcept(false) = 0;
    virtual const_iterator begin() const noexcept = 0;
    virtual const_iterator end() const noexcept = 0;
    virtual const value_type& Previous() const noexcept = 0;
    virtual std::size_t size() const noexcept = 0;
    virtual const value_type& Stop() const noexcept = 0;
    virtual filter::Type Type() const noexcept = 0;
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
    virtual OTData Element() const noexcept = 0;
};
struct Filterclear : virtual public bitcoin::Message {
};
struct Filterload : virtual public bitcoin::Message {
    virtual OTBloomFilter Filter() const noexcept = 0;
};
struct Getaddr : virtual public bitcoin::Message {
};
struct Getcfcheckpt : virtual public bitcoin::Message {
    virtual const filter::Hash& Stop() const noexcept = 0;
    virtual filter::Type Type() const noexcept = 0;
};
struct Getcfheaders : virtual public bitcoin::Message {
    virtual block::Height Start() const noexcept = 0;
    virtual const filter::Hash& Stop() const noexcept = 0;
    virtual filter::Type Type() const noexcept = 0;
};
struct Getcfilters : virtual public bitcoin::Message {
    virtual block::Height Start() const noexcept = 0;
    virtual const filter::Hash& Stop() const noexcept = 0;
    virtual filter::Type Type() const noexcept = 0;
};
struct Getdata : virtual public bitcoin::Message {
    using value_type = blockchain::bitcoin::Inventory;
    using const_iterator =
        iterator::Bidirectional<const Getdata, const value_type>;

    virtual const value_type& at(const std::size_t position) const
        noexcept(false) = 0;
    virtual const_iterator begin() const noexcept = 0;
    virtual const_iterator end() const noexcept = 0;
    virtual std::size_t size() const noexcept = 0;
};
struct Getheaders : virtual public bitcoin::Message {
    using value_type = block::Hash;
    using const_iterator =
        iterator::Bidirectional<const Getheaders, const value_type>;

    virtual const value_type& at(const std::size_t position) const
        noexcept(false) = 0;
    virtual const_iterator begin() const noexcept = 0;
    virtual const_iterator end() const noexcept = 0;
    virtual std::size_t size() const noexcept = 0;
    virtual block::pHash StopHash() const noexcept = 0;
    virtual ProtocolVersionUnsigned Version() const noexcept = 0;
};
struct Headers : virtual public bitcoin::Message {
    using value_type = block::bitcoin::Header;
    using const_iterator =
        iterator::Bidirectional<const Headers, const value_type>;

    virtual const value_type& at(const std::size_t position) const
        noexcept(false) = 0;
    virtual const_iterator begin() const noexcept = 0;
    virtual const_iterator end() const noexcept = 0;
    virtual std::size_t size() const noexcept = 0;
};
struct Inv : virtual public bitcoin::Message {
    using value_type = blockchain::bitcoin::Inventory;
    using const_iterator = iterator::Bidirectional<const Inv, const value_type>;

    virtual const value_type& at(const std::size_t position) const
        noexcept(false) = 0;
    virtual const_iterator begin() const noexcept = 0;
    virtual const_iterator end() const noexcept = 0;
    virtual std::size_t size() const noexcept = 0;
};
struct Mempool : virtual public bitcoin::Message {
};
struct Notfound : virtual public bitcoin::Message {
    using value_type = blockchain::bitcoin::Inventory;
    using const_iterator =
        iterator::Bidirectional<const Notfound, const value_type>;

    virtual const value_type& at(const std::size_t position) const
        noexcept(false) = 0;
    virtual const_iterator begin() const noexcept = 0;
    virtual const_iterator end() const noexcept = 0;
    virtual std::size_t size() const noexcept = 0;
};
struct Ping : virtual public bitcoin::Message {
    virtual bitcoin::Nonce Nonce() const noexcept = 0;
};
struct Pong : virtual public bitcoin::Message {
    virtual bitcoin::Nonce Nonce() const noexcept = 0;
};
struct Sendheaders : virtual public bitcoin::Message {
};
struct Verack : virtual public bitcoin::Message {
};
struct Version : virtual public bitcoin::Message {
    virtual block::Height Height() const noexcept = 0;
    virtual tcp::endpoint LocalAddress() const noexcept = 0;
    virtual std::set<blockchain::p2p::Service> LocalServices() const
        noexcept = 0;
    virtual api::client::blockchain::Nonce Nonce() const noexcept = 0;
    virtual bitcoin::ProtocolVersion ProtocolVersion() const noexcept = 0;
    virtual bool Relay() const noexcept = 0;
    virtual tcp::endpoint RemoteAddress() const noexcept = 0;
    virtual std::set<blockchain::p2p::Service> RemoteServices() const
        noexcept = 0;
    virtual const std::string& UserAgent() const noexcept = 0;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::internal
