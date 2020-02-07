// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Getcfcheckpt final : public internal::Getcfcheckpt
{
public:
    const filter::Hash& Stop() const noexcept final { return stop_; }
    filter::Type Type() const noexcept final { return type_; }

    ~Getcfcheckpt() final = default;

private:
    friend opentxs::Factory;

    using BitcoinFormat = FilterPrefixBasic;

    const filter::Type type_;
    const filter::pHash stop_;

    OTData payload() const noexcept final;

    Getcfcheckpt(
        const api::internal::Core& api,
        const blockchain::Type network,
        const filter::Type type,
        const filter::Hash& stop) noexcept;
    Getcfcheckpt(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const filter::Type type,
        const filter::Hash& stop) noexcept;
    Getcfcheckpt(const Getcfcheckpt&) = delete;
    Getcfcheckpt(Getcfcheckpt&&) = delete;
    Getcfcheckpt& operator=(const Getcfcheckpt&) = delete;
    Getcfcheckpt& operator=(Getcfcheckpt&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
