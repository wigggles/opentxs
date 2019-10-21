// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Cfilter final : public internal::Cfilter
{
public:
    GCS Filter() const noexcept final { return filter_; }
    const filter::Hash& Hash() const noexcept final { return hash_; }
    filter::Type Type() const noexcept final { return type_; }

    ~Cfilter() final = default;

private:
    friend opentxs::Factory;

    using BitcoinFormat = FilterPrefixBasic;

    static const std::map<filter::Type, std::uint32_t> gcs_bits_;
    static const std::map<filter::Type, std::uint32_t> gcs_fp_rate_;

    const filter::Type type_;
    const filter::pHash hash_;
    const std::shared_ptr<const blockchain::internal::GCS> filter_;

    OTData payload() const noexcept final;

    Cfilter(
        const api::internal::Core& api,
        const blockchain::Type network,
        const filter::Type type,
        const filter::Hash& hash,
        std::unique_ptr<blockchain::internal::GCS> filter) noexcept;
    Cfilter(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const filter::Type type,
        const filter::Hash& hash,
        std::unique_ptr<blockchain::internal::GCS> filter) noexcept;
    Cfilter(const Cfilter&) = delete;
    Cfilter(Cfilter&&) = delete;
    Cfilter& operator=(const Cfilter&) = delete;
    Cfilter& operator=(Cfilter&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
