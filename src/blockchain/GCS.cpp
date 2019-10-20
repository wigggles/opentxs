// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include "blockchain/bitcoin/CompactSize.hpp"
#include "internal/api/Api.hpp"
#include "internal/blockchain/Blockchain.hpp"

#include <boost/endian/buffers.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <array>
#include <cstdint>
#include <set>
#include <vector>

#include "GCS.hpp"

//#define OT_METHOD "opentxs::blockchain::implementation::GCS::"

namespace be = boost::endian;
namespace mp = boost::multiprecision;

#define BITMASK(n) ((1 << (n)) - 1)

namespace opentxs
{
blockchain::internal::GCS* Factory::GCS(
    const api::internal::Core& api,
    const std::uint32_t bits,
    const std::uint32_t fpRate,
    const std::array<std::byte, 16>& key,
    const std::vector<OTData>& elements)
{
    using ReturnType = blockchain::implementation::GCS;
    auto password = OTPassword{key.data(), key.size()};
    auto filter = ReturnType::build_gcs(api, bits, fpRate, password, elements);

    OTPassword pw{key.data(), key.size()};

    return new ReturnType(api, bits, fpRate, pw, elements);
}

blockchain::internal::GCS* Factory::GCS(
    const api::internal::Core& api,
    const proto::GCS& in)
{
    using ReturnType = blockchain::implementation::GCS;

    OTPassword pw{in.key().data(), in.key().size()};

    return new ReturnType(
        api,
        in.bits(),
        in.fprate(),
        pw,
        in.count(),
        Data::Factory(in.filter(), Data::Mode::Raw));
}

blockchain::internal::GCS* Factory::GCS(
    const api::internal::Core& api,
    const std::uint32_t bits,
    const std::uint32_t fpRate,
    const std::array<std::byte, 16>& key,
    const std::size_t filterElementCount,
    const Data& filter)
{
    using ReturnType = blockchain::implementation::GCS;

    OTPassword pw{key.data(), key.size()};

    return new ReturnType(
        api, bits, fpRate, pw, filterElementCount, Data::Factory(filter));
}
}  // namespace opentxs

namespace opentxs::blockchain::implementation
{
GCS::GCS(
    const api::internal::Core& api,
    const std::uint32_t bits,
    const std::uint32_t fpRate,
    OTPassword& key,
    const std::size_t filterElementCount,
    const Data& filter) noexcept
    : version_(1)
    , api_(api)
    , bits_(bits)
    , false_positive_rate_(fpRate)
    , key_(key)
    , filter_elements_(filterElementCount)
    , filter_((filter))
{
    if (key_.isPassword()) {
        OT_ASSERT(16 == key_.getPasswordSize());
    } else {
        OT_ASSERT(16 == key_.getMemorySize());
    }
}

GCS::GCS(
    const api::internal::Core& api,
    const std::uint32_t bits,
    const std::uint32_t fpRate,
    OTPassword& key,
    const std::vector<OTData>& elements) noexcept
    : GCS(api,
          bits,
          fpRate,
          key,
          elements.size(),
          build_gcs(api, bits, fpRate, key, elements))
{
}

OTData GCS::build_gcs(
    const api::internal::Core& api,
    const std::uint32_t bits,
    const std::uint32_t fpRate,
    const OTPassword& key,
    const std::vector<OTData>& elements) noexcept
{
    auto output = Data::Factory();

    const auto items =
        hashed_set_construct(api, fpRate, elements.size(), key, elements);

    BitWriter stream(output);
    std::uint64_t last_value{0};

    for (const auto& item : items) {
        std::uint64_t delta = item - last_value;

        if (delta != 0) { golomb_encode(bits, stream, delta); }
        last_value = item;
    }

    stream.flush();

    return output;
}

OTData GCS::Encode() const noexcept
{
    const auto bytes = bitcoin::CompactSize(filter_elements_).Encode();
    auto output = Data::Factory(bytes.data(), bytes.size());
    output += filter_;

    return output;
}

std::uint64_t GCS::golomb_decode(BitReader& stream) const noexcept
{
    std::uint64_t quotient{0};

    while (stream.read(1) == 1) { quotient++; }

    auto remainder = stream.read(bits_);
    std::uint64_t return_value = (quotient << bits_) + remainder;

    return return_value;
}

void GCS::golomb_encode(
    const std::uint32_t bits,
    BitWriter& stream,
    std::uint64_t value) noexcept
{
    // With Golomb-Rice, a value is split into a Quotient and Remainder modulo
    // 2^P, which are encoded separately.
    std::uint64_t remainder = value & BITMASK(bits);
    std::uint64_t quotient = (value - (remainder)) >> bits;

    // The quotient q is encoded as unary, with a string of q 1's followed by
    // one 0.
    while (quotient > 0) {
        stream.write(1, 1);
        quotient--;
    }

    stream.write(1, 0);
    // The remainder R is represented in big-endian by P bits.
    //
    // http://chinaober.com/github_/bitcoin/bips/commit/fcb7aeb7e28b78ecf6d145163e9559d01628e2d8
    // "The golomb_encode function should encode r instead of x."
    stream.write(bits, remainder);
}

std::uint64_t GCS::hash_to_range(
    const api::internal::Core& api,
    const OTPassword& key,
    const std::uint64_t maxRange,
    const Data& item) noexcept
{
    /*
     The items are first passed through the pseudorandom function SipHash, which
     takes a 128-bit key k and a variable-sized byte vector and produces a
     uniformly random 64-bit output. Implementations of this BIP MUST use the
     SipHash parameters c = 2 and d = 4.
     */
    mp::uint128_t siphash_128 = (siphash(api, key, item));
    mp::uint128_t maxrange_128 = (maxRange);
    mp::uint128_t multiplied = siphash_128 * maxrange_128;
    mp::uint128_t bitshifted = (multiplied >> 64);
    auto return64 = bitshifted.convert_to<std::uint64_t>();

    // let item = (siphash(key, target) * (N * (1 << fp))) >> 64
    // NOTICE The above commented code...we multiply the hash output
    // by (N * (1 << fp)) and then bitshift by 64.
    // See:
    // https://github.com/Roasbeef/bips/blob/master/gcs_light_client.mediawiki#Reference_Implementation
    //
    // But the original bip158 spec does this:
    // let F = N * M  (false positive rate of M)
    // return (siphash(k, item) * F) >> 64
    //
    // Notice the original code multiplies against the false positive rate,
    // and NOT against (1 << bit_parameter) !!!!!!
    //
    // One of them is presumably wrong??

    /*
     The 64-bit SipHash outputs are then mapped uniformly over the desired range
     by multiplying with F and taking the top 64 bits of the 128-bit result.

     This algorithm is a faster alternative to modulo reduction, as it avoids
     the expensive division operation[3]. Note that care must be taken when
     implementing this reduction to ensure the upper 64 bits of the integer
     multiplication are not truncated; certain architectures and high level
     languages may require code that decomposes the 64-bit multiplication into
     four 32-bit multiplications and recombines into the result.
     */

    return return64;
}

std::uint64_t GCS::hash_to_range(const Data& item, const std::uint64_t maxRange)
    const noexcept
{
    return hash_to_range(api_, key_, maxRange, item);
}

/*
 N is elementCount, the number of elements in the original set that created
   the filter.
 M is fpRate, the false probability rate. Customarily, M is set to 2^P.
 F is maxRange, which is N * M, that is, elementCount * fpRate.
 P is the bit length of the remainder code. It's the bits_ member variable.
 */
std::set<std::uint64_t> GCS::hashed_set_construct(
    const api::internal::Core& api,
    const std::uint32_t fpRate,
    const std::size_t elementCount,
    const OTPassword& key,
    const std::vector<OTData>& elements) noexcept
{
    // Original spec says: let F = N * M
    //  matches other items with probability 1/M for some integer parameter M
    // Other spec says: let F = N * P
    // P a value which is computed as 1/fp where fp is the desired false
    // positive rate.
    const std::uint64_t maxRange = elementCount * fpRate;
    std::set<std::uint64_t> set_items;  // hash values sorted ascending

    for (const auto& item : elements) {
        const std::uint64_t set_value = hash_to_range(api, key, maxRange, item);
        set_items.insert(set_value);
    }

    return set_items;
}

std::set<std::uint64_t> GCS::hashed_set_construct(
    const std::vector<OTData>& elements) const noexcept
{
    return hashed_set_construct(
        api_, false_positive_rate_, filter_elements_, key_, elements);
}

proto::GCS GCS::Serialize() const noexcept
{
    proto::GCS output{};
    output.set_version(version_);
    output.set_bits(bits_);
    output.set_fprate(false_positive_rate_);
    output.set_key(key_.getMemory(), key_.getMemorySize());
    output.set_count(static_cast<std::uint32_t>(filter_elements_));
    output.set_filter(filter_->str());

    return output;
}

std::uint64_t GCS::siphash(
    const api::internal::Core& api,
    const OTPassword& key,
    const Data& item) noexcept
{
    std::uint64_t output{};
    const bool hashed =
        api.Crypto().Hash().SipHash(key, item, output, siphash_c_, siphash_d_);

    if (hashed) { return output; }

    return 0;
}

std::uint64_t GCS::siphash(const Data& item) const noexcept
{
    return siphash(api_, key_, item);
}

bool GCS::Test(const Data& target) const noexcept
{
    const std::uint64_t max_range = filter_elements_ * false_positive_rate_;
    auto target_hash = hash_to_range(target, max_range);

    BitReader stream(filter_);
    std::uint64_t last_value{0};

    for (std::uint64_t i{0}; i < filter_elements_; ++i) {
        auto delta = golomb_decode(stream);
        auto set_item = last_value + delta;
        if (set_item == target_hash) { return true; }
        if (set_item > target_hash) { break; }
        last_value = set_item;
    }

    return false;
}

bool GCS::Test(const std::vector<OTData>& targets) const noexcept
{
    auto target_hashes_set = hashed_set_construct(targets);
    std::vector<std::uint64_t> target_hashes;

    for (const auto& target_hash : target_hashes_set) {
        target_hashes.push_back(target_hash);
    }

    BitReader stream(filter_);
    std::uint64_t value{0};
    std::size_t target_idx{0};
    std::uint64_t target_val = target_hashes[target_idx];

    for (std::size_t i{0}; i < filter_elements_; ++i) {
        auto delta = golomb_decode(stream);
        value += delta;
        if (target_val == value) {
            return true;
        } else if (target_val > value) {
            continue;
        } else {
            ++target_idx;

            if (target_idx == targets.size()) { break; }

            target_val = target_hashes[target_idx];
        }
    }

    return false;
}
}  // namespace opentxs::blockchain::implementation
