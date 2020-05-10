// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated

#include <map>
#include <string>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

const std::map<opentxs::blockchain::Type, opentxs::proto::ContactItemType>
    type_map_{
        {opentxs::blockchain::Type::Unknown, opentxs::proto::CITEMTYPE_UNKNOWN},
        {opentxs::blockchain::Type::Bitcoin, opentxs::proto::CITEMTYPE_BTC},
        {opentxs::blockchain::Type::Bitcoin_testnet3,
         opentxs::proto::CITEMTYPE_TNBTC},
        {opentxs::blockchain::Type::BitcoinCash, opentxs::proto::CITEMTYPE_BCH},
        {opentxs::blockchain::Type::BitcoinCash_testnet3,
         opentxs::proto::CITEMTYPE_TNBCH},
        {opentxs::blockchain::Type::Ethereum_frontier,
         opentxs::proto::CITEMTYPE_ETH},
        {opentxs::blockchain::Type::Ethereum_ropsten,
         opentxs::proto::CITEMTYPE_ETHEREUM_ROPSTEN},
        {opentxs::blockchain::Type::Litecoin, opentxs::proto::CITEMTYPE_LTC},
        {opentxs::blockchain::Type::Litecoin_testnet4,
         opentxs::proto::CITEMTYPE_TNLTC},
    };
const std::map<opentxs::proto::ContactItemType, opentxs::blockchain::Type>
    type_reverse_map_{opentxs::reverse_map(type_map_)};

namespace opentxs
{
auto operator==(
    const opentxs::ProtobufType& lhs,
    const opentxs::ProtobufType& rhs) noexcept -> bool
{
    auto sLeft = std::string{};
    auto sRight = std::string{};
    lhs.SerializeToString(&sLeft);
    rhs.SerializeToString(&sRight);

    return sLeft == sRight;
}

auto Translate(const blockchain::Type type) noexcept -> proto::ContactItemType
{
    try {
        return type_map_.at(type);
    } catch (...) {
        return proto::CITEMTYPE_UNKNOWN;
    }
}

auto Translate(const proto::ContactItemType type) noexcept -> blockchain::Type
{
    try {
        return type_reverse_map_.at(type);
    } catch (...) {
        return blockchain::Type::Unknown;
    }
}
}  // namespace opentxs
