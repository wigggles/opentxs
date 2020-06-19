// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "util/Container.hpp"

namespace opentxs::blockchain::p2p::bitcoin
{
const std::map<blockchain::Type, std::map<bitcoin::Service, p2p::Service>>
    service_bit_map_{
        {blockchain::Type::Bitcoin,
         {
             {bitcoin::Service::None, p2p::Service::None},
             {bitcoin::Service::Bit1, p2p::Service::Network},
             {bitcoin::Service::Bit2, p2p::Service::UTXO},
             {bitcoin::Service::Bit3, p2p::Service::Bloom},
             {bitcoin::Service::Bit4, p2p::Service::Witness},
             {bitcoin::Service::Bit5, p2p::Service::XThin},
             {bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {bitcoin::Service::Bit7, p2p::Service::CompactFilters},
             {bitcoin::Service::Bit8, p2p::Service::Segwit2X},
             {bitcoin::Service::Bit11, p2p::Service::Limited},
         }},
        {blockchain::Type::Bitcoin_testnet3,
         {
             {bitcoin::Service::None, p2p::Service::None},
             {bitcoin::Service::Bit1, p2p::Service::Network},
             {bitcoin::Service::Bit2, p2p::Service::UTXO},
             {bitcoin::Service::Bit3, p2p::Service::Bloom},
             {bitcoin::Service::Bit4, p2p::Service::Witness},
             {bitcoin::Service::Bit5, p2p::Service::XThin},
             {bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {bitcoin::Service::Bit7, p2p::Service::CompactFilters},
             {bitcoin::Service::Bit8, p2p::Service::Segwit2X},
             {bitcoin::Service::Bit11, p2p::Service::Limited},
         }},
        {blockchain::Type::BitcoinCash,
         {
             {bitcoin::Service::None, p2p::Service::None},
             {bitcoin::Service::Bit1, p2p::Service::Network},
             {bitcoin::Service::Bit2, p2p::Service::UTXO},
             {bitcoin::Service::Bit3, p2p::Service::Bloom},
             {bitcoin::Service::Bit4, p2p::Service::Witness},
             {bitcoin::Service::Bit5, p2p::Service::XThin},
             {bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {bitcoin::Service::Bit7, p2p::Service::Graphene},
             {bitcoin::Service::Bit8, p2p::Service::WeakBlocks},
             {bitcoin::Service::Bit9, p2p::Service::CompactFilters},
             {bitcoin::Service::Bit10, p2p::Service::XThinner},
             {bitcoin::Service::Bit11, p2p::Service::Limited},
             {bitcoin::Service::Bit25, p2p::Service::Avalanche},
         }},
        {blockchain::Type::BitcoinCash_testnet3,
         {
             {bitcoin::Service::None, p2p::Service::None},
             {bitcoin::Service::Bit1, p2p::Service::Network},
             {bitcoin::Service::Bit2, p2p::Service::UTXO},
             {bitcoin::Service::Bit3, p2p::Service::Bloom},
             {bitcoin::Service::Bit4, p2p::Service::Witness},
             {bitcoin::Service::Bit5, p2p::Service::XThin},
             {bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
             {bitcoin::Service::Bit7, p2p::Service::Graphene},
             {bitcoin::Service::Bit8, p2p::Service::WeakBlocks},
             {bitcoin::Service::Bit9, p2p::Service::CompactFilters},
             {bitcoin::Service::Bit10, p2p::Service::XThinner},
             {bitcoin::Service::Bit11, p2p::Service::Limited},
             {bitcoin::Service::Bit25, p2p::Service::Avalanche},
         }},
    };
const std::map<blockchain::Type, std::map<p2p::Service, bitcoin::Service>>
    service_bit_reverse_map_{reverse_nested_map(service_bit_map_)};

const MagicMap network_map_{
    {blockchain::Type::Bitcoin, Magic::Bitcoin},
    {blockchain::Type::Bitcoin_testnet3, Magic::BTCTestnet3},
    {blockchain::Type::BitcoinCash, Magic::BitcoinCash},
    {blockchain::Type::BitcoinCash_testnet3, Magic::BCHTestnet3},
};
const MagicReverseMap network_reverse_map_{reverse_map(network_map_)};
const CommandMap command_map_{
    {Command::addr, "addr"},
    {Command::alert, "alert"},
    {Command::block, "block"},
    {Command::blocktxn, "blocktxn"},
    {Command::cfcheckpt, "cfcheckpt"},
    {Command::cfheaders, "cfheaders"},
    {Command::cfilter, "cfilter"},
    {Command::checkorder, "checkorder"},
    {Command::cmpctblock, "cmpctblock"},
    {Command::feefilter, "feefilter"},
    {Command::filteradd, "filteradd"},
    {Command::filterclear, "filterclear"},
    {Command::filterload, "filterload"},
    {Command::getaddr, "getaddr"},
    {Command::getblocks, "getblocks"},
    {Command::getblocktxn, "getblocktxn"},
    {Command::getcfcheckpt, "getcfcheckpt"},
    {Command::getcfheaders, "getcfheaders"},
    {Command::getcfilters, "getcfilters"},
    {Command::getdata, "getdata"},
    {Command::getheaders, "getheaders"},
    {Command::headers, "headers"},
    {Command::inv, "inv"},
    {Command::mempool, "mempool"},
    {Command::merkleblock, "merkleblock"},
    {Command::notfound, "notfound"},
    {Command::ping, "ping"},
    {Command::pong, "pong"},
    {Command::reject, "reject"},
    {Command::reply, "reply"},
    {Command::sendcmpct, "sendcmpct"},
    {Command::sendheaders, "sendheaders"},
    {Command::submitorder, "submitorder"},
    {Command::tx, "tx"},
    {Command::verack, "verack"},
    {Command::version, "version"},
};
const CommandReverseMap command_reverse_map_{reverse_map(command_map_)};
const std::map<std::uint32_t, Magic> magic_map_{
    {static_cast<std::uint32_t>(Magic::BTCTestnet3), Magic::BTCTestnet3},
    {static_cast<std::uint32_t>(Magic::Bitcoin), Magic::Bitcoin},
    {static_cast<std::uint32_t>(Magic::BitcoinCash), Magic::BitcoinCash},
    {static_cast<std::uint32_t>(Magic::BCHTestnet3), Magic::BCHTestnet3},
};

const OTData AddressVersion::cjdns_prefix_{
    Data::Factory("0xfc", Data::Mode::Hex)};
const OTData AddressVersion::ipv4_prefix_{
    Data::Factory("0x00000000000000000000ffff", Data::Mode::Hex)};
const OTData AddressVersion::onion_prefix_{
    Data::Factory("0xfd87d87eeb43", Data::Mode::Hex)};

AddressVersion::AddressVersion(
    const std::set<bitcoin::Service>& services,
    const tcp::endpoint& endpoint) noexcept
    : services_(GetServiceBytes(services))
    , address_(endpoint.address().to_v6().to_bytes())
    , port_(endpoint.port())
{
    static_assert(26 == sizeof(AddressVersion));
}

AddressVersion::AddressVersion(
    const blockchain::Type chain,
    const ProtocolVersion version,
    const internal::Address& address) noexcept
    : services_(GetServiceBytes(
          TranslateServices(chain, version, address.Services())))
    , address_(Encode(address.Type(), address.Bytes()))
    , port_(address.Port())
{
    static_assert(26 == sizeof(AddressVersion));
}

AddressVersion::AddressVersion() noexcept
    : services_()
    , address_()
    , port_()
{
    static_assert(26 == sizeof(AddressVersion));
}

auto AddressVersion::Encode(const Network type, const Data& bytes)
    -> AddressByteField
{
    AddressByteField output{};

    switch (type) {
        case Network::ipv6:
        case Network::cjdns: {
            OT_ASSERT(output.size() == bytes.size());

            std::memcpy(output.data(), bytes.data(), output.size());
        } break;
        case Network::ipv4: {
            auto encoded{ipv4_prefix_};
            encoded += bytes;

            OT_ASSERT(output.size() == encoded->size());

            std::memcpy(output.data(), encoded->data(), output.size());
        } break;
        case Network::onion2: {
            auto encoded{onion_prefix_};
            encoded += bytes;

            OT_ASSERT(output.size() == encoded->size());

            std::memcpy(output.data(), encoded->data(), output.size());
        } break;
        case Network::onion3:
        case Network::eep:
        default: {
            OT_FAIL;
        }
    }

    return output;
}

auto BitcoinString(const std::string& in) noexcept -> OTData
{
    const auto size = CompactSize(in.size()).Encode();
    auto output = Data::Factory(size.data(), size.size());

    if (false == in.empty()) { output->Concatenate(in.data(), in.size()); }

    return output;
}

auto CommandName(const Command command) noexcept -> std::string
{
    try {
        return command_map_.at(command);
    } catch (...) {
        return {};
    }
}

auto convert_service_bit(BitVector8 value) noexcept -> bitcoin::Service
{
    if (0 == value) { return Service::None; }

    auto log{1};

    while (value >>= 1) { ++log; }

    return static_cast<bitcoin::Service>(log);
}

auto convert_service_bit(const bitcoin::Service value) noexcept -> BitVector8
{
    if (bitcoin::Service::None == value) { return {}; }

    BitVector8 output{1u};
    output <<= (static_cast<std::uint8_t>(value) - 1);

    return output;
}

auto GetCommand(const CommandField& bytes) noexcept -> Command
{
    try {
        const std::string raw{
            reinterpret_cast<const char*>(bytes.data()), bytes.size()};
        const auto command = std::string{raw.c_str()};

        return command_reverse_map_.at(command);
    } catch (...) {

        return Command::unknown;
    }
}

auto GetMagic(const blockchain::Type type) noexcept -> Magic
{
    try {
        return network_map_.at(type);
    } catch (...) {
        return Magic::Unknown;
    }
}

auto GetMagic(const std::uint32_t magic) noexcept -> Magic
{
    try {
        return magic_map_.at(magic);
    } catch (...) {
        return Magic::Unknown;
    }
}

auto GetNetwork(const Magic magic) noexcept -> blockchain::Type
{
    try {
        return network_reverse_map_.at(magic);
    } catch (...) {
        return blockchain::Type::Unknown;
    }
}

auto GetServiceBytes(const std::set<bitcoin::Service>& services) noexcept
    -> BitVector8
{
    BitVector8 output{0};

    for (const auto& bit : services) {
        if (Service::None == bit) { continue; }

        output |= convert_service_bit(bit);
    }

    return output;
}

auto GetServices(const BitVector8 data) noexcept -> std::set<bitcoin::Service>
{
    if (0 == data) { return {}; }

    std::set<bitcoin::Service> output{};
    BitVector8 mask{1};

    for (std::size_t i = 0; i < (8 * sizeof(data)); ++i) {
        const auto value = data & mask;

        if (0 != value) { output.emplace(convert_service_bit(value)); }

        mask *= 2;
    }

    return output;
}

auto SerializeCommand(const Command command) noexcept -> CommandField
{
    CommandField output{};

    try {
        const auto string = CommandName(command);

        OT_ASSERT(output.size() >= string.size());

        std::memcpy(output.data(), string.data(), string.size());
    } catch (...) {
    }

    return output;
}

auto TranslateServices(
    const blockchain::Type chain,
    [[maybe_unused]] const ProtocolVersion version,
    const std::set<p2p::Service>& input) noexcept -> std::set<bitcoin::Service>
{
    std::set<bitcoin::Service> output{};
    std::for_each(
        std::begin(input),
        std::end(input),
        [&output, chain](const auto& in) -> void {
            try {
                output.emplace(service_bit_reverse_map_.at(chain).at(in));
            } catch (...) {
            }
        });

    return output;
}

auto TranslateServices(
    const blockchain::Type chain,
    [[maybe_unused]] const ProtocolVersion version,
    const std::set<bitcoin::Service>& input) noexcept -> std::set<p2p::Service>
{
    std::set<p2p::Service> output{};
    std::for_each(
        std::begin(input),
        std::end(input),
        [&output, chain](const auto& in) -> void {
            try {
                output.emplace(service_bit_map_.at(chain).at(in));
            } catch (...) {
            }
        });

    return output;
}
}  // namespace opentxs::blockchain::p2p::bitcoin
