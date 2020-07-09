// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/protobuf/GCS.pb.h"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace blockchain
{
namespace block
{
class Block;
}  // namespace block

class BloomFilter;
class NumericHash;
class Work;
}  // namespace blockchain
}  // namespace opentxs

namespace be = boost::endian;

namespace opentxs::gcs
{
OPENTXS_EXPORT auto GolombDecode(
    const std::uint32_t N,
    const std::uint8_t P,
    const Space& encoded) noexcept(false) -> std::vector<std::uint64_t>;
OPENTXS_EXPORT auto GolombEncode(
    const std::uint8_t P,
    const std::vector<std::uint64_t>& hashedSet) noexcept(false) -> Space;
OPENTXS_EXPORT auto HashToRange(
    const api::Core& api,
    const ReadView key,
    const std::uint64_t range,
    const ReadView item) noexcept(false) -> std::uint64_t;
OPENTXS_EXPORT auto HashedSetConstruct(
    const api::Core& api,
    const ReadView key,
    const std::uint32_t N,
    const std::uint32_t M,
    const std::vector<ReadView> items) noexcept(false)
    -> std::vector<std::uint64_t>;
}  // namespace opentxs::gcs

namespace opentxs::blockchain::internal
{
// Source of BitReader class:
// https://github.com/rasky/gcs/blob/master/cpp/gcs.cpp
// The license there reads:
// "This is free and unencumbered software released into the public domain."
class BitReader
{
public:
    OPENTXS_EXPORT auto eof() -> bool;
    OPENTXS_EXPORT auto read(std::size_t nbits) -> std::uint64_t;

    OPENTXS_EXPORT BitReader(const Space& data);
    OPENTXS_EXPORT BitReader(std::uint8_t* data, int len);

private:
    OTData raw_data_;
    std::uint8_t* data_{nullptr};
    std::size_t len_{};
    std::uint64_t accum_{};
    std::size_t n_{};

    BitReader() = delete;
    BitReader(const BitReader&) = delete;
    BitReader(BitReader&&) = delete;
    auto operator=(const BitReader&) -> BitReader& = delete;
    auto operator=(BitReader &&) -> BitReader& = delete;
};

// Source of BitWriter class:
// https://github.com/rasky/gcs/blob/master/cpp/gcs.cpp
// The license there reads:
// "This is free and unencumbered software released into the public domain."
class BitWriter
{
public:
    OPENTXS_EXPORT void flush();
    OPENTXS_EXPORT void write(std::size_t nbits, std::uint64_t value);

    OPENTXS_EXPORT BitWriter(Space& output);

private:
    enum { ACCUM_BITS = sizeof(std::uint64_t) * 8 };

    Space& output_;
    std::uint64_t accum_{};
    std::size_t n_{};

    BitWriter() = delete;
};

struct GCS {
    using Targets = std::vector<ReadView>;
    using Matches = std::vector<Targets::const_iterator>;

    virtual auto Compressed() const noexcept -> Space = 0;
    virtual auto ElementCount() const noexcept -> std::uint32_t = 0;
    virtual auto Encode() const noexcept -> OTData = 0;
    virtual auto Hash() const noexcept -> OTData = 0;
    virtual auto Header(const ReadView previous) const noexcept -> OTData = 0;
    virtual auto Match(const Targets&) const noexcept -> Matches = 0;
    virtual auto Serialize() const noexcept -> proto::GCS = 0;
    virtual auto Test(const Data& target) const noexcept -> bool = 0;
    virtual auto Test(const ReadView target) const noexcept -> bool = 0;
    virtual auto Test(const std::vector<OTData>& targets) const noexcept
        -> bool = 0;
    virtual auto Test(const std::vector<Space>& targets) const noexcept
        -> bool = 0;

    virtual ~GCS() = default;
};

struct SerializedBloomFilter {
    be::little_uint32_buf_t function_count_;
    be::little_uint32_buf_t tweak_;
    be::little_uint8_buf_t flags_;

    SerializedBloomFilter(
        const std::uint32_t tweak,
        const BloomUpdateFlag update,
        const std::size_t functionCount) noexcept;
    SerializedBloomFilter() noexcept;
};

struct Database : virtual public client::internal::BlockDatabase,
                  virtual public client::internal::FilterDatabase,
                  virtual public client::internal::HeaderDatabase,
                  virtual public client::internal::PeerDatabase,
                  virtual public client::internal::WalletDatabase {

    virtual ~Database() = default;
};

using FilterParams = std::pair<std::uint8_t, std::uint32_t>;

auto DefaultFilter(const Type type) noexcept -> filter::Type;
auto DecodeSerializedCfilter(const ReadView bytes) noexcept(false)
    -> std::pair<std::uint32_t, ReadView>;
auto Deserialize(const Type chain, const std::uint8_t type) noexcept
    -> filter::Type;
auto Deserialize(const api::Core& api, const ReadView bytes) noexcept
    -> block::Position;
auto DisplayString(const Type type) noexcept -> std::string;
OPENTXS_EXPORT auto BlockHashToFilterKey(const ReadView hash) noexcept(false)
    -> ReadView;
OPENTXS_EXPORT auto FilterHashToHeader(
    const api::Core& api,
    const ReadView hash,
    const ReadView previous = {}) noexcept -> OTData;
OPENTXS_EXPORT auto FilterToHash(
    const api::Core& api,
    const ReadView filter) noexcept -> OTData;
OPENTXS_EXPORT auto FilterToHeader(
    const api::Core& api,
    const ReadView filter,
    const ReadView previous = {}) noexcept -> OTData;
auto Format(const Type chain, const opentxs::Amount) noexcept -> std::string;
OPENTXS_EXPORT auto GetFilterParams(const filter::Type type) noexcept(false)
    -> FilterParams;
OPENTXS_EXPORT
auto Grind(const std::function<void()> function) noexcept -> void;
auto Serialize(const Type chain, const filter::Type type) noexcept(false)
    -> std::uint8_t;
auto Serialize(const block::Position& position) noexcept -> Space;
auto Ticker(const Type chain) noexcept -> std::string;
}  // namespace opentxs::blockchain::internal

namespace opentxs::blockchain::script
{
}  // namespace opentxs::blockchain::script

namespace opentxs::factory
{
#if OT_BLOCKCHAIN
OPENTXS_EXPORT auto BloomFilter(
    const api::Core& api,
    const std::uint32_t tweak,
    const blockchain::BloomUpdateFlag update,
    const std::size_t targets,
    const double falsePositiveRate) -> blockchain::BloomFilter*;
OPENTXS_EXPORT auto BloomFilter(const api::Core& api, const Data& serialized)
    -> blockchain::BloomFilter*;
OPENTXS_EXPORT auto GCS(
    const api::Core& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const ReadView key,
    const std::vector<OTData>& elements) noexcept
    -> std::unique_ptr<blockchain::internal::GCS>;
OPENTXS_EXPORT auto GCS(
    const api::Core& api,
    const blockchain::filter::Type type,
    const blockchain::block::Block& block) noexcept
    -> std::unique_ptr<blockchain::internal::GCS>;
OPENTXS_EXPORT auto GCS(
    const api::Core& api,
    const proto::GCS& serialized) noexcept
    -> std::unique_ptr<blockchain::internal::GCS>;
OPENTXS_EXPORT auto GCS(
    const api::Core& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const ReadView key,
    const std::uint32_t filterElementCount,
    const ReadView filter) noexcept
    -> std::unique_ptr<blockchain::internal::GCS>;
OPENTXS_EXPORT auto GCS(
    const api::Core& api,
    const blockchain::filter::Type type,
    const ReadView key,
    const ReadView encoded) noexcept
    -> std::unique_ptr<blockchain::internal::GCS>;
OPENTXS_EXPORT auto NumericHash(const blockchain::block::Hash& hash) noexcept
    -> std::unique_ptr<blockchain::NumericHash>;
OPENTXS_EXPORT auto NumericHashNBits(const std::int32_t nBits) noexcept
    -> std::unique_ptr<blockchain::NumericHash>;
OPENTXS_EXPORT auto Work(const std::string& hex) -> blockchain::Work*;
OPENTXS_EXPORT auto Work(const blockchain::NumericHash& target)
    -> blockchain::Work*;
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::factory
