// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include "internal/api/Api.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/core/Core.hpp"
#include "blockchain/block/Header.hpp"

#include <boost/endian/buffers.hpp>

#include <array>

#include "Header.hpp"

#define OT_BITCOIN_BLOCK_HEADER_SIZE 80

#define OT_METHOD "opentxs::blockchain::block::bitcoin::Header::"

namespace opentxs
{
blockchain::block::bitcoin::internal::Header* Factory::BitcoinBlockHeader(
    const api::internal::Core& api,
    const proto::BlockchainBlockHeader& serialized)
{
    using ReturnType = blockchain::block::bitcoin::implementation::Header;

    return new ReturnType(api, serialized);
}

blockchain::block::bitcoin::internal::Header* Factory::BitcoinBlockHeader(
    const api::internal::Core& api,
    const Data& raw)
{
    using ReturnType = blockchain::block::bitcoin::implementation::Header;

    if (OT_BITCOIN_BLOCK_HEADER_SIZE != raw.size()) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Invalid serialized block")
            .Flush();

        return nullptr;
    }

    static_assert(
        OT_BITCOIN_BLOCK_HEADER_SIZE == sizeof(ReturnType::BitcoinFormat));

    ReturnType::BitcoinFormat serialized{};
    const auto result = OTPassword::safe_memcpy(
        &serialized, sizeof(serialized), raw.data(), raw.size());

    if (nullptr == result) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Failed to deserialize header")
            .Flush();

        return nullptr;
    }

    return new ReturnType(
        api,
        ReturnType::subversion_default_,
        ReturnType::calculate_hash(api, raw),
        serialized.version_.value(),
        Data::Factory(serialized.previous_.data(), serialized.previous_.size()),
        Data::Factory(serialized.merkle_.data(), serialized.merkle_.size()),
        Clock::from_time_t(std::time_t(serialized.time_.value())),
        serialized.nbits_.value(),
        serialized.nonce_.value());
}

blockchain::block::bitcoin::internal::Header* Factory::BitcoinBlockHeader(
    const api::internal::Core& api,
    const blockchain::block::Hash& hash,
    const blockchain::block::Hash& parent,
    const blockchain::block::Height height)
{
    using ReturnType = blockchain::block::bitcoin::implementation::Header;

    return new ReturnType(api, hash, parent, height);
}
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
Header::Header(
    const api::internal::Core& api,
    const VersionNumber subversion,
    const block::Hash& hash,
    const std::int32_t version,
    const block::Hash& previous,
    const block::Hash& merkle,
    const Time timestamp,
    const std::int32_t nbits,
    const std::uint32_t nonce) noexcept
    : bitcoin::Header()
    , ot_super(
          api,
          blockchain::Type::Bitcoin,
          hash,
          previous,
          make_blank<block::Height>::value(api),
          calculate_work(nbits))
    , subversion_(subversion)
    , block_version_(version)
    , merkle_root_(merkle)
    , timestamp_(timestamp)
    , nbits_(nbits)
    , nonce_(nonce)
{
}

Header::Header(
    const api::internal::Core& api,
    const block::Hash& hash,
    const block::Hash& parentHash,
    const block::Height height) noexcept
    : bitcoin::Header()
    , ot_super(
          api,
          blockchain::Type::Bitcoin,
          hash,
          parentHash,
          height,
          minimum_work())
    , subversion_(subversion_default_)
    , block_version_(0)
    , merkle_root_(hash)
    , timestamp_(make_blank<Time>::value(api))
    , nbits_(NumericHash::MaxTarget)
    , nonce_(0)
{
    const_cast<OTData&>(hash_) = calculate_hash(api, Serialize());
}

Header::Header(
    const api::internal::Core& api,
    const SerializedType& serialized) noexcept
    : bitcoin::Header()
    , ot_super(
          api,
          calculate_hash(api, serialized),
          Data::Factory(
              serialized.bitcoin().previous_header(),
              Data::Mode::Raw),
          serialized)
    , subversion_(serialized.bitcoin().version())
    , block_version_(serialized.bitcoin().block_version())
    , merkle_root_(
          Data::Factory(serialized.bitcoin().merkle_hash(), Data::Mode::Raw))
    , timestamp_(Clock::from_time_t(serialized.bitcoin().timestamp()))
    , nbits_(serialized.bitcoin().nbits())
    , nonce_(serialized.bitcoin().nonce())
{
}

Header::Header(const Header& rhs) noexcept
    : bitcoin::Header()
    , ot_super(rhs)
    , subversion_(rhs.subversion_)
    , block_version_(rhs.block_version_)
    , merkle_root_(rhs.merkle_root_)
    , timestamp_(rhs.timestamp_)
    , nbits_(rhs.nbits_)
    , nonce_(rhs.nonce_)
{
}

Header::BitcoinFormat::BitcoinFormat() noexcept
    : version_()
    , previous_()
    , merkle_()
    , time_()
    , nbits_()
    , nonce_()
{
    static_assert(80 == sizeof(BitcoinFormat));
}

Header::BitcoinFormat::BitcoinFormat(
    const std::int32_t version,
    const std::string& previous,
    const std::string& merkle,
    const std::uint32_t time,
    const std::int32_t nbits,
    const std::uint32_t nonce) noexcept(false)
    : version_(version)
    , previous_()
    , merkle_()
    , time_(time)
    , nbits_(nbits)
    , nonce_(nonce)
{
    static_assert(80 == sizeof(BitcoinFormat));

    if (sizeof(previous_) < previous.size()) {
        throw std::invalid_argument("Invalid previous hash");
    }

    if (sizeof(merkle_) < merkle.size()) {
        throw std::invalid_argument("Invalid merkle hash");
    }

    std::memcpy(previous_.data(), previous.data(), previous.size());
    std::memcpy(merkle_.data(), merkle.data(), merkle.size());
}

block::pHash Header::calculate_hash(
    const api::internal::Core& api,
    const opentxs::Data& serialized)
{
    auto intermediate = Data::Factory();
    api.Crypto().Hash().Digest(
        proto::HASHTYPE_SHA256, serialized, intermediate);
    auto output = Data::Factory();
    api.Crypto().Hash().Digest(proto::HASHTYPE_SHA256, intermediate, output);

    return output;
}

block::pHash Header::calculate_hash(
    const api::internal::Core& api,
    const SerializedType& serialized)
{
    BitcoinFormat bitcoinFormat{};

    try {
        bitcoinFormat = BitcoinFormat{serialized.bitcoin().block_version(),
                                      serialized.bitcoin().previous_header(),
                                      serialized.bitcoin().merkle_hash(),
                                      serialized.bitcoin().timestamp(),
                                      serialized.bitcoin().nbits(),
                                      serialized.bitcoin().nonce()};
    } catch (const std::invalid_argument& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();
    }

    return calculate_hash(
        api, Data::Factory(&bitcoinFormat, sizeof(bitcoinFormat)));
}

OTWork Header::calculate_work(const std::int32_t nbits)
{
    const auto hash = OTNumericHash{Factory::NumericHashNBits(nbits)};

    return OTWork{Factory::Work(hash)};
}

std::unique_ptr<block::Header> Header::clone() const noexcept
{
    return std::unique_ptr<block::Header>(new Header(*this));
}

OTData Header::Encode() const noexcept
{
    BitcoinFormat raw{block_version_,
                      parent_hash_->str(),
                      merkle_root_->str(),
                      static_cast<std::uint32_t>(Clock::to_time_t(timestamp_)),
                      nbits_,
                      nonce_};

    return Data::Factory(&raw, sizeof(raw));
}

Header::SerializedType Header::Serialize() const noexcept
{
    auto output = ot_super::Serialize();
    auto& bitcoin = *output.mutable_bitcoin();
    bitcoin.set_version(subversion_);
    bitcoin.set_block_version(block_version_);
    bitcoin.set_previous_header(parent_hash_->str());
    bitcoin.set_merkle_hash(merkle_root_->str());
    bitcoin.set_timestamp(Clock::to_time_t(timestamp_));
    bitcoin.set_nbits(nbits_);
    bitcoin.set_nonce(nonce_);

    return output;
}

OTNumericHash Header::Target() const noexcept
{
    return OTNumericHash{Factory::NumericHashNBits(nbits_)};
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
