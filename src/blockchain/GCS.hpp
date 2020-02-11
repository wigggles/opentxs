// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::implementation
{
class GCS final : virtual public internal::GCS
{
public:
    OTData Encode() const noexcept final;
    OTData Hash() const noexcept final;
    proto::GCS Serialize() const noexcept final;
    bool Test(const Data& target) const noexcept final;
    bool Test(const std::vector<OTData>& targets) const noexcept final;

    ~GCS() final = default;

private:
    friend opentxs::Factory;

    using BitReader = internal::BitReader;
    using BitWriter = internal::BitWriter;

    static const int siphash_c_{2};
    static const int siphash_d_{4};

    const VersionNumber version_;
    const api::internal::Core& api_;
    const std::uint32_t bits_;
    const std::uint32_t false_positive_rate_;
    const OTData key_;
    const std::size_t filter_elements_;
    const OTData filter_;

    static OTData build_gcs(
        const api::internal::Core& api,
        const std::uint32_t bits,
        const std::uint32_t fpRate,
        const Data& key,
        const std::vector<OTData>& elements) noexcept;
    static void golomb_encode(
        const std::uint32_t bits,
        BitWriter& stream,
        std::uint64_t delta) noexcept;
    static std::uint64_t hash_to_range(
        const api::internal::Core& api,
        const Data& key,
        const std::uint64_t maxRange,
        const Data& item) noexcept;
    static std::set<std::uint64_t> hashed_set_construct(
        const api::internal::Core& api,
        const std::uint32_t fpRate,
        const std::size_t elementCount,
        const Data& key,
        const std::vector<OTData>& elements) noexcept;
    static std::uint64_t siphash(
        const api::internal::Core& api,
        const Data& key,
        const Data& item) noexcept;

    std::uint64_t golomb_decode(BitReader& stream) const noexcept;
    std::uint64_t siphash(const Data& item) const noexcept;
    std::uint64_t hash_to_range(const Data& item, const std::uint64_t maxRange)
        const noexcept;
    std::set<std::uint64_t> hashed_set_construct(
        const std::vector<OTData>& elements) const noexcept;

    GCS(const api::internal::Core& api,
        const std::uint32_t bits,
        const std::uint32_t fpRate,
        const Data& key,
        const std::size_t filterElementCount,
        const Data& filter)
    noexcept(false);
    GCS(const api::internal::Core& api,
        const std::uint32_t bits,
        const std::uint32_t fpRate,
        const Data& key,
        const std::vector<OTData>& elements)
    noexcept(false);
    GCS() = delete;
    GCS(const GCS&) = delete;
    GCS(GCS&&) = delete;
    GCS& operator=(const GCS&) = delete;
    GCS& operator=(GCS&&) = delete;
};
}  // namespace opentxs::blockchain::implementation
