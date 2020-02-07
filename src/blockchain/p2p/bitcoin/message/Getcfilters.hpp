// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Getcfilters final : public internal::Getcfilters
{
public:
    block::Height Start() const noexcept final { return start_; }
    const filter::Hash& Stop() const noexcept final { return stop_; }
    filter::Type Type() const noexcept final { return type_; }

    ~Getcfilters() final = default;

private:
    friend opentxs::Factory;

    using BitcoinFormat = FilterRequest;

    const filter::Type type_;
    const block::Height start_;
    const filter::pHash stop_;

    OTData payload() const noexcept final;

    Getcfilters(
        const api::internal::Core& api,
        const blockchain::Type network,
        const filter::Type type,
        const block::Height start,
        const filter::Hash& stop) noexcept;
    Getcfilters(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const filter::Type type,
        const block::Height start,
        const filter::Hash& stop) noexcept;
    Getcfilters(const Getcfilters&) = delete;
    Getcfilters(Getcfilters&&) = delete;
    Getcfilters& operator=(const Getcfilters&) = delete;
    Getcfilters& operator=(Getcfilters&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
