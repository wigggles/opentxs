// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/block/bitcoin/Header.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstring>
#include <ctime>
#include <stdexcept>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "util/Blank.hpp"

#define OT_BITCOIN_BLOCK_HEADER_SIZE 80

#define OT_METHOD "opentxs::blockchain::block::bitcoin::Header::"

namespace opentxs::factory
{
auto BitcoinBlockHeader(
    const api::client::Manager& api,
    const proto::BlockchainBlockHeader& serialized) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Header;

    return std::make_unique<ReturnType>(api, serialized);
}

auto BitcoinBlockHeader(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const ReadView raw) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Header;

    if (OT_BITCOIN_BLOCK_HEADER_SIZE != raw.size()) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(
            ": Invalid serialized block")
            .Flush();

        return nullptr;
    }

    static_assert(
        OT_BITCOIN_BLOCK_HEADER_SIZE == sizeof(ReturnType::BitcoinFormat));

    auto serialized = ReturnType::BitcoinFormat{};

    OT_ASSERT(sizeof(serialized) == raw.size());

    const auto result =
        std::memcpy(static_cast<void*>(&serialized), raw.data(), raw.size());

    if (nullptr == result) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(
            ": Failed to deserialize header")
            .Flush();

        return nullptr;
    }

    return std::make_unique<ReturnType>(
        api,
        chain,
        ReturnType::subversion_default_,
        ReturnType::calculate_hash(api, chain, raw),
        serialized.version_.value(),
        Data::Factory(serialized.previous_.data(), serialized.previous_.size()),
        Data::Factory(serialized.merkle_.data(), serialized.merkle_.size()),
        Clock::from_time_t(std::time_t(serialized.time_.value())),
        serialized.nbits_.value(),
        serialized.nonce_.value());
}

auto BitcoinBlockHeader(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const blockchain::block::Hash& hash,
    const blockchain::block::Hash& parent,
    const blockchain::block::Height height) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Header;

    return std::make_unique<ReturnType>(api, chain, hash, parent, height);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin::implementation
{
const VersionNumber Header::local_data_version_{1};
const VersionNumber Header::subversion_default_{1};

Header::Header(
    const api::client::Manager& api,
    const blockchain::Type chain,
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
          chain,
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
    const api::client::Manager& api,
    const blockchain::Type chain,
    const block::Hash& hash,
    const block::Hash& parentHash,
    const block::Height height) noexcept
    : bitcoin::Header()
    , ot_super(api, chain, hash, parentHash, height, minimum_work())
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
    const api::client::Manager& api,
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

auto Header::calculate_hash(
    const api::client::Manager& api,
    const blockchain::Type chain,
    const ReadView serialized) -> block::pHash
{
    auto output = api.Factory().Data();
    BlockHash(api, chain, serialized, output->WriteInto());

    return output;
}

auto Header::calculate_hash(
    const api::client::Manager& api,
    const SerializedType& serialized) -> block::pHash
{
    auto bitcoinFormat = BitcoinFormat{};

    try {
        bitcoinFormat = BitcoinFormat{
            serialized.bitcoin().block_version(),
            serialized.bitcoin().previous_header(),
            serialized.bitcoin().merkle_hash(),
            serialized.bitcoin().timestamp(),
            serialized.bitcoin().nbits(),
            serialized.bitcoin().nonce()};
    } catch (const std::invalid_argument& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();
    }

    return calculate_hash(
        api,
        static_cast<blockchain::Type>(serialized.type()),
        ReadView(
            reinterpret_cast<const char*>(&bitcoinFormat),
            sizeof(bitcoinFormat)));
}

auto Header::calculate_work(const std::int32_t nbits) -> OTWork
{
    const auto hash = OTNumericHash{factory::NumericHashNBits(nbits)};

    return OTWork{factory::Work(hash)};
}

auto Header::clone() const noexcept -> std::unique_ptr<block::Header>
{
    return std::unique_ptr<block::Header>(new Header(*this));
}

auto Header::Encode() const noexcept -> OTData
{
    auto output = api_.Factory().Data();
    Serialize(output->WriteInto());

    return output;
}

auto Header::Serialize() const noexcept -> Header::SerializedType
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

auto Header::Serialize(const AllocateOutput destination) const noexcept -> bool
{
    const auto raw = BitcoinFormat{
        block_version_,
        parent_hash_->str(),
        merkle_root_->str(),
        static_cast<std::uint32_t>(Clock::to_time_t(timestamp_)),
        nbits_,
        nonce_};

    if (false == bool(destination)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();

        return false;
    }

    const auto out = destination(sizeof(raw));

    if (false == out.valid(sizeof(raw))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to allocate output")
            .Flush();

        return false;
    }

    std::memcpy(out.data(), &raw, sizeof(raw));

    return true;
}

auto Header::Target() const noexcept -> OTNumericHash
{
    return OTNumericHash{factory::NumericHashNBits(nbits_)};
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
