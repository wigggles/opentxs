// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/Proto.hpp"

#include "internal/blockchain/client/Client.hpp"

#include <boost/endian/buffers.hpp>

#include <cstdint>
#include <string>
#include <tuple>

namespace be = boost::endian;

namespace opentxs::blockchain::internal
{
// Source of BitReader class:
// https://github.com/rasky/gcs/blob/master/cpp/gcs.cpp
// The license there reads:
// "This is free and unencumbered software released into the public domain."
class BitReader
{
public:
    bool eof();
    std::uint64_t read(std::size_t nbits);

    BitReader(const Data& input_data);
    BitReader(std::uint8_t* data, int len);

private:
    OTData raw_data_;
    std::uint8_t* data_{nullptr};
    std::size_t len_{};
    std::uint64_t accum_{};
    std::size_t n_{};

    BitReader() = delete;
    BitReader(const BitReader&) = delete;
    BitReader(BitReader&&) = delete;
    BitReader& operator=(const BitReader&) = delete;
    BitReader& operator=(BitReader&&) = delete;
};

// Source of BitWriter class:
// https://github.com/rasky/gcs/blob/master/cpp/gcs.cpp
// The license there reads:
// "This is free and unencumbered software released into the public domain."
class BitWriter
{
public:
    void flush();
    void write(std::size_t nbits, std::uint64_t value);

    BitWriter(Data& output);

private:
    enum { ACCUM_BITS = sizeof(std::uint64_t) * 8 };

    Data& output_;
    std::uint64_t accum_{};
    std::size_t n_{};

    BitWriter() = delete;
};

struct GCS {
    virtual OTData Encode() const noexcept = 0;
    virtual proto::GCS Serialize() const noexcept = 0;
    virtual bool Test(const Data& target) const noexcept = 0;
    virtual bool Test(const std::vector<OTData>& targets) const noexcept = 0;

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

struct Database : virtual public client::internal::FilterDatabase,
                  virtual public client::internal::HeaderDatabase,
                  virtual public client::internal::PeerDatabase {

    virtual ~Database() = default;
};

std::string DisplayString(const Type type) noexcept;
}  // namespace opentxs::blockchain::internal
