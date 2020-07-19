// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "internal/blockchain/Params.hpp"  // IWYU pragma: associated

#include <memory>
#include <set>
#include <type_traits>

#include "opentxs/Bytes.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/protobuf/Enums.pb.h"

namespace opentxs::blockchain
{
auto BlockHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        default: {
            return api.Crypto().Hash().Digest(
                proto::HASHTYPE_SHA256D, input, output);
        }
    }
}

auto FilterHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        default: {
            return api.Crypto().Hash().Digest(
                proto::HASHTYPE_SHA256D, input, output);
        }
    }
}

auto P2PMessageHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        default: {
            return api.Crypto().Hash().Digest(
                proto::HASHTYPE_SHA256DC, input, output);
        }
    }
}

auto ProofOfWorkHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        default: {
            return BlockHash(api, chain, input, output);
        }
    }
}

auto PubkeyHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        default: {
            return api.Crypto().Hash().Digest(
                proto::HASHTYPE_BITCOIN, input, output);
        }
    }
}

auto ScriptHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        default: {
            return api.Crypto().Hash().Digest(
                proto::HASHTYPE_BITCOIN, input, output);
        }
    }
}

auto SupportedChains() noexcept -> const std::set<Type>&
{
    static const auto build = []() -> auto
    {
        auto output = std::set<Type>{};

        for (const auto& [chain, data] : params::Data::chains_) {
            if (data.supported_) { output.emplace(chain); }
        }

        return output;
    };

    static const auto output{build()};

    return output;
}

auto TransactionHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool
{
    switch (chain) {
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        default: {
            return api.Crypto().Hash().Digest(
                proto::HASHTYPE_SHA256D, input, output);
        }
    }
}
}  // namespace opentxs::blockchain

namespace opentxs::blockchain::params
{
const Data::ChainData Data::chains_{
    {blockchain::Type::Bitcoin,
     {true,
      opentxs::proto::CITEMTYPE_BTC,
      "Bitcoin",
      "BTC",
      8,
      486604799,  // 0x1d00ffff
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a29ab5f49"
      "ffff001d1dac2b7c",
      "6fe28c0ab6f1b372c1a6a246ae63f74f931e8365e15a089c68d6190000000000",
      {630000,
       "6de9d737a62ea1c197000edb02c252089969dfd8ea4b02000000000000000000",
       "1e2b96b120a73bacb0667279bd231bdb95b08be16b650d000000000000000000",
       "63059e205633ebffb7d35e737611a6be5d1d3f904fa9c86a756afa7e0aee02f2"},
      filter::Type::Basic_BIP158,
      p2p::Protocol::bitcoin,
      3652501241,
      8333,
      {
          "seed.bitcoin.sipa.be",
          "dnsseed.bluematt.me",
          "dnsseed.bitcoin.dashjr.org",
          "seed.bitcoinstats.com",
          "seed.bitcoin.jonasschnelli.ch",
          "seed.btc.petertodd.org",
          "seed.bitcoin.sprovoost.nl",
          "dnsseed.emzy.de",
      },
      25000}},
    {blockchain::Type::Bitcoin_testnet3,
     {true,
      opentxs::proto::CITEMTYPE_TNBTC,
      "Bitcoin (testnet3)",
      "tnBTC",
      8,
      486604799,  // 0x1d00ffff
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4adae5494d"
      "ffff001d1aa4ae18",
      "43497fd7f826957108f4a30fd9cec3aeba79972084e90ead01ea330900000000",
      {1740000,
       "f758e6307382affd044c64e2bead2efdd2d9222bce9d232ae3fc000000000000",
       "b7b063b8908b78d83c837c53bfa1222dc141fab18537d525f5f6000000000000",
       "6ae92c333eafd91b2a995064f249fe558ea7569fdc723b5e008637362829c1e0"},
      filter::Type::Basic_BIP158,
      p2p::Protocol::bitcoin,
      118034699,
      18333,
      {
          "testnet-seed.bitcoin.jonasschnelli.ch",
          "seed.tbtc.petertodd.org",
          "seed.testnet.bitcoin.sprovoost.nl",
          "testnet-seed.bluematt.me",
          "testnet-seed.bitcoin.schildbach.de",
      },
      3113}},
    {blockchain::Type::BitcoinCash,
     {true,
      opentxs::proto::CITEMTYPE_BCH,
      "Bitcoin Cash",
      "BCH",
      8,
      486604799,  // 0x1d00ffff
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a29ab5f49"
      "ffff001d1dac2b7c",
      "6fe28c0ab6f1b372c1a6a246ae63f74f931e8365e15a089c68d6190000000000",
      {635259,
       "f73075b2c598f49b3a19558c070b52d5a5d6c21fefdf33000000000000000000",
       "1a78f5c89b05a56167066be9b0a36ac8f1781ed0472c30030000000000000000",
       "15ceed5cc33ecfdda722164b81b43e4e95ba7e1d65eaf94f7c2e3707343bf4c5"},
      filter::Type::Basic_BCHVariant,
      p2p::Protocol::bitcoin,
      3908297187,
      8333,
      {
          "seed.bitcoinabc.org",
          "seed-abc.bitcoinforks.org",
          "btccash-seeder.bitcoinunlimited.info",
          "seed.deadalnix.me",
          "seed.bchd.cash",
          "dnsseed.electroncash.de",
      },
      1000}},
    {blockchain::Type::BitcoinCash_testnet3,
     {true,
      opentxs::proto::CITEMTYPE_TNBCH,
      "Bitcoin Cash (testnet3)",
      "tnBCH",
      8,
      486604799,  // 0x1d00ffff
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4adae5494d"
      "ffff001d1aa4ae18",
      "43497fd7f826957108f4a30fd9cec3aeba79972084e90ead01ea330900000000",
      {1378461,
       "d715e9fab7bbdf301081eeadbe6e931db282cf6b92b1365f9b50f59900000000",
       "24b33d026d36cbff3a693ea754a3be177dc5fb80966294cb643cf37000000000",
       "ee4ac1f50b0fba7dd9d2c5145fb4dd6a7e66b516c08a9b505cad8c2da53263fa"},
      filter::Type::Basic_BCHVariant,
      p2p::Protocol::bitcoin,
      4109624820,
      18333,
      {
          "testnet-seed.bitcoinabc.org",
          "testnet-seed-abc.bitcoinforks.org",
          "testnet-seed.bitprim.org",
          "testnet-seed.deadalnix.me",
          "testnet-seed.bchd.cash",
      },
      1000}},
    {blockchain::Type::Ethereum_frontier,
     {false,
      opentxs::proto::CITEMTYPE_ETH,
      "Ethereum (frontier)",
      "",
      0,
      0,
      "",
      "d4e56740f876aef8c010b86a40d5f56745a118d0906a34e69aec8c0db1cb8fa3",
      {0, "", "", ""},
      {},
      p2p::Protocol::ethereum,
      0,
      30303,
      {},
      0}},
    {blockchain::Type::Ethereum_ropsten,
     {false,
      opentxs::proto::CITEMTYPE_ETHEREUM_ROPSTEN,
      "Ethereumn (ropsten testnet)",
      "",
      0,
      0,
      "",
      "41941023680923e0fe4d74a34bdac8141f2540e3ae90623718e47d66d1ca4a2d",
      {0, "", "", ""},
      {},
      p2p::Protocol::ethereum,
      0,
      30303,
      {},
      0}},
    {blockchain::Type::Litecoin,
     {false,
      opentxs::proto::CITEMTYPE_LTC,
      "",
      "",
      0,
      0,
      "",
      "",
      {0, "", "", ""},
      {},
      p2p::Protocol::bitcoin,
      0,
      0,
      {},
      0}},
    {blockchain::Type::Litecoin_testnet4,
     {false,
      opentxs::proto::CITEMTYPE_TNLTC,
      "",
      "",
      0,
      0,
      "",
      "",
      {0, "", "", ""},
      {},
      p2p::Protocol::bitcoin,
      0,
      0,
      {},
      0}},
};

const Data::FilterData Data::genesis_filters_{
    {blockchain::Type::Bitcoin,
     {
         {filter::Type::Basic_BIP158,
          {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c202",
           "017fa880"}},
         {filter::Type::Extended_opentxs,
          {"0354578634dd178058ad5f3addf0d97c45911f483c99a1022ce51502e142e99f",
           "049dc75e0d584a300293ef3d3980"}},
     }},
    {blockchain::Type::BitcoinCash,
     {
         {filter::Type::Basic_BCHVariant,
          {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c202",
           "017fa880"}},
         {filter::Type::Extended_opentxs,
          {"0354578634dd178058ad5f3addf0d97c45911f483c99a1022ce51502e142e99"
           "f",
           "049dc75e0d584a300293ef3d3980"}},
     }},
    {blockchain::Type::Bitcoin_testnet3,
     {
         {filter::Type::Basic_BIP158,
          {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb779455821",
           "019dfca8"}},
         {filter::Type::Extended_opentxs,
          {"a1310188d76ce653283a3086aa6f1ba30b6934990a093e1789a78a43b926131"
           "5",
           "04e2f587e146bf6c662d35278a40"}},
     }},
    {blockchain::Type::BitcoinCash_testnet3,
     {
         {filter::Type::Basic_BCHVariant,
          {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb779455821",
           "019dfca8"}},
         {filter::Type::Extended_opentxs,
          {"a1310188d76ce653283a3086aa6f1ba30b6934990a093e1789a78a43b926131"
           "5",
           "04e2f587e146bf6c662d35278a40"}},
     }},
};

const Data::FilterTypes Data::bip158_types_{
    {Type::Bitcoin,
     {
         {filter::Type::Basic_BIP158, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::Bitcoin_testnet3,
     {
         {filter::Type::Basic_BIP158, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::BitcoinCash,
     {
         {filter::Type::Basic_BCHVariant, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::BitcoinCash_testnet3,
     {
         {filter::Type::Basic_BCHVariant, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::Ethereum_frontier, {}},
    {Type::Ethereum_ropsten, {}},
    {Type::Litecoin,
     {
         {filter::Type::Basic_BIP158, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
    {Type::Litecoin_testnet4,
     {
         {filter::Type::Basic_BIP158, 0x0},
         {filter::Type::Extended_opentxs, 0x58},
     }},
};

const Data::ServiceBits Data::service_bits_{
    {blockchain::Type::Bitcoin,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
     }},
    {blockchain::Type::Bitcoin_testnet3,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit8, p2p::Service::Segwit2X},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
     }},
    {blockchain::Type::BitcoinCash,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::Graphene},
         {p2p::bitcoin::Service::Bit8, p2p::Service::WeakBlocks},
         {p2p::bitcoin::Service::Bit9, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit10, p2p::Service::XThinner},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
         {p2p::bitcoin::Service::Bit25, p2p::Service::Avalanche},
     }},
    {blockchain::Type::BitcoinCash_testnet3,
     {
         {p2p::bitcoin::Service::None, p2p::Service::None},
         {p2p::bitcoin::Service::Bit1, p2p::Service::Network},
         {p2p::bitcoin::Service::Bit2, p2p::Service::UTXO},
         {p2p::bitcoin::Service::Bit3, p2p::Service::Bloom},
         {p2p::bitcoin::Service::Bit4, p2p::Service::Witness},
         {p2p::bitcoin::Service::Bit5, p2p::Service::XThin},
         {p2p::bitcoin::Service::Bit6, p2p::Service::BitcoinCash},
         {p2p::bitcoin::Service::Bit7, p2p::Service::Graphene},
         {p2p::bitcoin::Service::Bit8, p2p::Service::WeakBlocks},
         {p2p::bitcoin::Service::Bit9, p2p::Service::CompactFilters},
         {p2p::bitcoin::Service::Bit10, p2p::Service::XThinner},
         {p2p::bitcoin::Service::Bit11, p2p::Service::Limited},
         {p2p::bitcoin::Service::Bit25, p2p::Service::Avalanche},
     }},
};
}  // namespace opentxs::blockchain::params
