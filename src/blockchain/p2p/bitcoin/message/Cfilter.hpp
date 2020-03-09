// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Cfilter final : public internal::Cfilter
{
public:
    auto Bits() const noexcept -> std::uint8_t final
    {
        return gcs_bits_.at(type_);
    }
    auto ElementCount() const noexcept -> std::uint32_t final { return count_; }
    auto FPRate() const noexcept -> std::uint32_t final
    {
        return gcs_fp_rate_.at(type_);
    }
    auto Filter() const noexcept -> ReadView { return reader(filter_); }
    auto Hash() const noexcept -> const filter::Hash& final { return hash_; }
    auto Type() const noexcept -> filter::Type final { return type_; }

    ~Cfilter() final = default;

private:
    friend opentxs::Factory;

    using BitcoinFormat = FilterPrefixBasic;

    static const std::map<filter::Type, std::uint8_t> gcs_bits_;
    static const std::map<filter::Type, std::uint32_t> gcs_fp_rate_;

    const filter::Type type_;
    const filter::pHash hash_;
    const std::uint32_t count_;
    const Space filter_;

    OTData payload() const noexcept final;

    Cfilter(
        const api::internal::Core& api,
        const blockchain::Type network,
        const filter::Type type,
        const filter::Hash& hash,
        const std::uint32_t count,
        const Space& compressed) noexcept;
    Cfilter(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const filter::Type type,
        const filter::Hash& hash,
        const std::uint32_t count,
        Space&& compressed) noexcept;
    Cfilter(const Cfilter&) = delete;
    Cfilter(Cfilter&&) = delete;
    Cfilter& operator=(const Cfilter&) = delete;
    Cfilter& operator=(Cfilter&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
