// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include <array>

#if OT_CRYPTO_SUPPORTED_KEY_HD
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
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

namespace opentxs
{
bool operator==(
    const opentxs::ProtobufType& lhs,
    const opentxs::ProtobufType& rhs) noexcept
{
    auto sLeft = std::string{};
    auto sRight = std::string{};
    lhs.SerializeToString(&sLeft);
    rhs.SerializeToString(&sRight);

    return sLeft == sRight;
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
proto::ContactItemType Translate(const blockchain::Type type) noexcept
{
    try {
        return type_map_.at(type);
    } catch (...) {
        return proto::CITEMTYPE_UNKNOWN;
    }
}

blockchain::Type Translate(const proto::ContactItemType type) noexcept
{
    try {
        return type_reverse_map_.at(type);
    } catch (...) {
        return blockchain::Type::Unknown;
    }
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
}  // namespace opentxs
