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

#if OT_BLOCKCHAIN
#include "opentxs/Bytes.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
#endif  // OT_BLOCKCHAIN

#if OT_BLOCKCHAIN
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
        case Type::UnitTest:
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
        case Type::UnitTest:
        default: {
            return BlockHash(api, chain, input, output);
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
        case Type::UnitTest:
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
        case Type::Litecoin:
        case Type::Litecoin_testnet4: {
            return api.Crypto().Hash().Scrypt(
                input, input, 1024, 1, 1, 32, output);
        }
        case Type::UnitTest:
        case Type::Unknown:
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
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
        case Type::UnitTest:
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
        case Type::UnitTest:
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
        case Type::UnitTest:
        default: {
            return BlockHash(api, chain, input, output);
        }
    }
}
}  // namespace opentxs::blockchain
#endif  // OT_BLOCKCHAIN

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
      {1775000,
       "ee03a42e71e08b0ec11f4a41b3dd49eb033a02d91c9e62607c00000000000000",
       "e34b7a89fd357ab76c784ad25dce9751aec2147407b520e9a21a9f2a00000000",
       "7a9e7964ffdf67007431fc592d3debd5e12a43dbf55a140ad51dfe5caae6ada1"},
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
      "Ethereum (ropsten testnet)",
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
     {true,
      opentxs::proto::CITEMTYPE_LTC,
      "Litecoin",
      "LTC",
      8,
      504365040,  // 0x1e0ffff0
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "d9ced4ed1130f7b7faad9be25323ffafa33232a17c3edf6cfd97bee6bafbdd97b9aa8e4e"
      "f0ff0f1ecd513f7c",
      "e2bf047e7e5a191aa4ef34d314979dc9986e0f19251edaba5940fd1fe365a712",
      {1725000,
       "75086858fbea5c6782ff65a4eb5fa338e9d930d0320786abc707df00d30f41e3",
       "7014a9b68e409dddf51892d63f84183476268768a77fcbfb9ce11e4e28ad2610",
       "8bf7e9466b1be7a01a5106fe18b1402964e54fe843a92365f3f9012489b35859"},
      filter::Type::Basic_BIP158,
      p2p::Protocol::bitcoin,
      3686187259,
      9333,
      {
          "seed-a.litecoin.loshan.co.uk",
          "dnsseed.thrasher.io",
          "dnsseed.litecointools.com",
          "dnsseed.litecoinpool.org",
      },
      25000}},
    {blockchain::Type::Litecoin_testnet4,
     {true,
      opentxs::proto::CITEMTYPE_TNLTC,
      "Litecoin (testnet4)",
      "tnLTC",
      8,
      504365040,  // 0x1e0ffff0
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "d9ced4ed1130f7b7faad9be25323ffafa33232a17c3edf6cfd97bee6bafbdd97f60ba158"
      "f0ff0f1ee1790400",
      "a0293e4eeb3da6e6f56f81ed595f57880d1a21569e13eefdd951284b5a626649",
      {1550000,
       "8aef627831d5128695fcda8bc34b0d9780ad8f58d7a869f31f54af3acbbe7fff",
       "8295f3dcb3fea7b3536e6b8ed598c056edc6c5a23bc4efaf821086b56f24dda2",
       "2081dcdef3c741e20033a348a35d1b2952f31a9eee2285ea4f5cdb8c1c8decfe"},
      filter::Type::Basic_BIP158,
      p2p::Protocol::bitcoin,
      4056470269,
      19335,
      {
          "testnet-seed.litecointools.com",
          "seed-b.litecoin.loshan.co.uk",
          "dnsseed-testnet.thrasher.io",
      },
      25000}},
    {blockchain::Type::UnitTest,
     {false,
      opentxs::proto::CITEMTYPE_ERROR,
      "Unit Test Simulation",
      "UNITTEST",
      8,
      545259519,  // 0x207fffff
      "010000000000000000000000000000000000000000000000000000000000000000000000"
      "3ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4adae5494d"
      "ffff7f2002000000",
      "06226e46111a0b59caaf126043eb5bbf28c34f3a5e332a1fc7b2b73cf188910f",
      {630000,
       "6de9d737a62ea1c197000edb02c252089969dfd8ea4b02000000000000000000",
       "1e2b96b120a73bacb0667279bd231bdb95b08be16b650d000000000000000000",
       "63059e205633ebffb7d35e737611a6be5d1d3f904fa9c86a756afa7e0aee02f2"},
      filter::Type::Basic_BIP158,
      p2p::Protocol::bitcoin,
      3669344250,
      18444,
      {},
      0}},
};

#if OT_BLOCKCHAIN
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
          {"0354578634dd178058ad5f3addf0d97c45911f483c99a1022ce51502e142e99f",
           "049dc75e0d584a300293ef3d3980"}},
     }},
    {blockchain::Type::Litecoin,
     {
         {filter::Type::Basic_BIP158,
          {"8aa75530308cf8247a151c37c24e7aaa281ae3b5cecedb581aacb3a0d07c2451",
           "019e8738"}},
         {filter::Type::Extended_opentxs,
          {"5a71cc36ad0b35d4c99b335ff69c3ed89e667b9772dbbf40396a1b4f2b2c3080",
           "049de8963322099e81f3bf7c4600"}},
     }},
    {blockchain::Type::Bitcoin_testnet3,
     {
         {filter::Type::Basic_BIP158,
          {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb779455821",
           "019dfca8"}},
         {filter::Type::Extended_opentxs,
          {"a1310188d76ce653283a3086aa6f1ba30b6934990a093e1789a78a43b9261315",
           "04e2f587e146bf6c662d35278a40"}},
     }},
    {blockchain::Type::BitcoinCash_testnet3,
     {
         {filter::Type::Basic_BCHVariant,
          {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb779455821",
           "019dfca8"}},
         {filter::Type::Extended_opentxs,
          {"a1310188d76ce653283a3086aa6f1ba30b6934990a093e1789a78a43b9261315",
           "04e2f587e146bf6c662d35278a40"}},
     }},
    {blockchain::Type::Litecoin_testnet4,
     {
         {filter::Type::Basic_BIP158,
          {"02d023da9d271b849f717089aad7e03a515dac982c9fb2cfd952e2ce1c618792",
           "014c8c60"}},
         {filter::Type::Extended_opentxs,
          {"042bce138093a271d8d7f730f7f9f9ab8c7240f297b47aea4440dceec623aca3",
           "048b3d6095a4b01eb30ce44017c0"}},
     }},
    {blockchain::Type::UnitTest,
     {
         {filter::Type::Basic_BIP158,
          {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c202",
           "017fa880"}},
         {filter::Type::Extended_opentxs,
          {"0354578634dd178058ad5f3addf0d97c45911f483c99a1022ce51502e142e99f",
           "049dc75e0d584a300293ef3d3980"}},
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
    {Type::UnitTest,
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
    {blockchain::Type::Litecoin,
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
    {blockchain::Type::Litecoin_testnet4,
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
    {blockchain::Type::UnitTest,
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
};
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::blockchain::params
