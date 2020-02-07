// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/dynamic_bitset.hpp>

#include <memory>

namespace be = boost::endian;

namespace opentxs::blockchain::implementation
{
class BloomFilter final : virtual public blockchain::BloomFilter
{
public:
    OTData Serialize() const noexcept final;
    bool Test(const Data& element) const noexcept final;

    void AddElement(const Data& element) noexcept final;

    ~BloomFilter() final = default;

private:
    friend opentxs::Factory;

    using FalsePositiveRate = double;
    using Filter = boost::dynamic_bitset<>;
    using Tweak = std::uint32_t;

    static const std::size_t max_filter_bytes_;
    static const std::size_t max_hash_function_count_;
    static const std::uint32_t seed_;

    const api::internal::Core& api_;
    Tweak tweak_{};
    BloomUpdateFlag flags_{};
    std::size_t function_count_{};
    Filter filter_;

    BloomFilter* clone() const noexcept final { return new BloomFilter(*this); }
    std::uint32_t hash(const Data& input, std::size_t hash_index) const
        noexcept;

    BloomFilter(
        const api::internal::Core& api,
        const Tweak tweak,
        const BloomUpdateFlag update,
        const std::size_t functionCount,
        const Filter& data) noexcept;
    BloomFilter(
        const api::internal::Core& api,
        const Tweak tweak,
        const BloomUpdateFlag update,
        const std::size_t targets,
        const FalsePositiveRate rate) noexcept;
    BloomFilter(
        const api::internal::Core& api,
        const Tweak tweak,
        const BloomUpdateFlag update,
        const std::size_t functionCount,
        const Data& data) noexcept;
    BloomFilter() = delete;
    BloomFilter(const BloomFilter& rhs) noexcept;
    BloomFilter(BloomFilter&&) = delete;
    BloomFilter& operator=(const BloomFilter&) = delete;
    BloomFilter& operator=(BloomFilter&&) = delete;
};
}  // namespace opentxs::blockchain::implementation
