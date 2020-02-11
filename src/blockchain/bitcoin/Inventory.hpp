// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/Data.hpp"

#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"

#include <cstdint>
#include <map>
#include <string>

namespace opentxs::blockchain::bitcoin
{
class Inventory
{
public:
    enum class Type : std::uint8_t {
        None = 0,
        MsgTx = 1,
        MsgBlock = 2,
        MsgFilteredBlock = 3,
        MsgCmpctBlock = 4,
        MsgWitnessTx = 5,
        MsgWitnessBlock = 6,
        MsgFilteredWitnessBlock = 7
    };

    static const std::size_t EncodedSize;

    const Type type_;
    const pHash hash_;

    static std::string DisplayType(const Type type) noexcept;

    std::string DisplayType() const noexcept { return DisplayType(type_); }
    OTData Encode() const noexcept;

    Inventory(const Type type, const Hash& hash) noexcept;
    Inventory(const void* payload, const std::size_t size) noexcept;
    Inventory(Inventory&&);

    ~Inventory() = default;

private:
    using Map = std::map<Type, std::uint32_t>;
    using ReverseMap = std::map<std::uint32_t, Type>;

    struct BitcoinFormat {
        p2p::bitcoin::message::InventoryTypeField type_;
        p2p::bitcoin::message::HashField hash_;

        BitcoinFormat(const Type type, const Hash& hash) noexcept(false);
    };

    static const Map map_;
    static const ReverseMap reverse_map_;

    static OTData decode_hash(
        const void* payload,
        const std::size_t size) noexcept(false);
    static Type decode_type(
        const void* payload,
        const std::size_t size) noexcept(false);
    static std::uint32_t encode_type(const Type type) noexcept(false);
    static p2p::bitcoin::message::HashField encode_hash(
        const Hash& hash) noexcept(false);

    Inventory() = delete;
    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;
    Inventory& operator=(Inventory&&) = delete;
};
}  // namespace opentxs::blockchain::bitcoin
