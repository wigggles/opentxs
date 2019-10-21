// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Frame.hpp"

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/api/Api.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"

namespace opentxs::blockchain::p2p::bitcoin::message
{
const std::map<filter::Type, std::uint8_t> filter_type_map_{
    {filter::Type::Basic, 0},
    {filter::Type::Undefined, 1},
};
const std::map<std::uint8_t, filter::Type> filter_type_reverse_map_{
    reverse_map(filter_type_map_)};

filter::Type DeserializeClientFilterType(const std::uint8_t type) noexcept
{
    try {
        return filter_type_reverse_map_.at(type);
    } catch (...) {
        return filter::Type::Unknown;
    }
}

FilterPrefixBasic::FilterPrefixBasic(
    const filter::Type type,
    const filter::Hash& hash) noexcept(false)
    : type_(SerializeClientFilterType(type))
    , hash_()
{
    static_assert(33 == sizeof(FilterPrefixBasic));

    if (hash.size() != hash_.size()) {
        throw std::runtime_error("Invalid hash");
    }

    std::memcpy(hash_.data(), hash.data(), hash_.size());
}

FilterPrefixBasic::FilterPrefixBasic() noexcept
    : type_()
    , hash_()
{
    static_assert(33 == sizeof(FilterPrefixBasic));
}

filter::pHash FilterPrefixBasic::Hash() const noexcept
{
    return Data::Factory(hash_.data(), hash_.size());
}

filter::Type FilterPrefixBasic::Type() const noexcept
{
    return DeserializeClientFilterType(type_.value());
}

FilterPrefixChained::FilterPrefixChained(
    const filter::Type type,
    const filter::Hash& hash,
    const filter::Hash& previous) noexcept(false)
    : type_(SerializeClientFilterType(type))
    , hash_()
    , previous_()
{
    static_assert(65 == sizeof(FilterPrefixChained));

    if (hash.size() != hash_.size()) {
        throw std::runtime_error("Invalid stop hash");
    }

    if (previous.size() != previous_.size()) {
        throw std::runtime_error("Invalid previous hash");
    }

    std::memcpy(hash_.data(), hash.data(), hash_.size());
    std::memcpy(previous_.data(), previous.data(), previous_.size());
}

FilterPrefixChained::FilterPrefixChained() noexcept
    : type_()
    , hash_()
    , previous_()
{
    static_assert(65 == sizeof(FilterPrefixChained));
}

filter::pHash FilterPrefixChained::Previous() const noexcept
{
    return Data::Factory(previous_.data(), previous_.size());
}

filter::pHash FilterPrefixChained::Stop() const noexcept
{
    return Data::Factory(hash_.data(), hash_.size());
}

filter::Type FilterPrefixChained::Type() const noexcept
{
    return DeserializeClientFilterType(type_.value());
}

FilterRequest::FilterRequest(
    const filter::Type type,
    const block::Height start,
    const filter::Hash& stop) noexcept(false)
    : type_(SerializeClientFilterType(type))
    , start_(start)
    , stop_()
{
    static_assert(37 == sizeof(FilterRequest));

    if (stop.size() != stop_.size()) {
        throw std::runtime_error("Invalid stop hash");
    }

    std::memcpy(stop_.data(), stop.data(), stop_.size());
}

FilterRequest::FilterRequest() noexcept
    : type_()
    , start_()
    , stop_()
{
    static_assert(37 == sizeof(FilterRequest));
}

block::Height FilterRequest::Start() const noexcept { return start_.value(); }

filter::pHash FilterRequest::Stop() const noexcept
{
    return Data::Factory(stop_.data(), stop_.size());
}

filter::Type FilterRequest::Type() const noexcept
{
    return DeserializeClientFilterType(type_.value());
}

std::uint8_t SerializeClientFilterType(const filter::Type type) noexcept
{
    return filter_type_map_.at(type);
}

bool VerifyChecksum(
    const api::internal::Core& api,
    const Header& header,
    const network::zeromq::Frame& payload) noexcept
{
    auto checksum = Data::Factory();

    switch (header.Network()) {
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3: {
            if (0 == payload.size()) {
                checksum = Data::Factory("0x5df6e0e2", Data::Mode::Hex);
            } else {
                auto intermediate = Data::Factory();
                api.Crypto().Hash().Digest(
                    proto::HASHTYPE_SHA256,
                    payload.data(),
                    payload.size(),
                    intermediate);
                auto hash = Data::Factory();
                api.Crypto().Hash().Digest(
                    proto::HASHTYPE_SHA256, intermediate, hash);
                hash->Extract(4, checksum);
            }
        } break;
        default: {
            LogOutput(__FUNCTION__)(": Unsupported type").Flush();
        }
    }

    return header.Checksum() == checksum;
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message
