// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Cfcheckpt final : public internal::Cfcheckpt
{
public:
    const value_type& at(const std::size_t position) const noexcept(false) final
    {
        return payload_.at(position);
    }
    const_iterator begin() const noexcept final
    {
        return const_iterator(this, 0);
    }
    const_iterator end() const noexcept final
    {
        return const_iterator(this, payload_.size());
    }
    std::size_t size() const noexcept final { return payload_.size(); }
    const value_type& Stop() const noexcept final { return stop_; }
    filter::Type Type() const noexcept final { return type_; }

    ~Cfcheckpt() final = default;

private:
    friend opentxs::Factory;

    using BitcoinFormat = FilterPrefixBasic;

    const filter::Type type_;
    const filter::pHash stop_;
    const std::vector<filter::pHash> payload_;

    OTData payload() const noexcept final;

    Cfcheckpt(
        const api::internal::Core& api,
        const blockchain::Type network,
        const filter::Type type,
        const filter::Hash& stop,
        const std::vector<filter::pHash>& headers) noexcept;
    Cfcheckpt(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const filter::Type type,
        const filter::Hash& stop,
        const std::vector<filter::pHash>& headers) noexcept;
    Cfcheckpt(const Cfcheckpt&) = delete;
    Cfcheckpt(Cfcheckpt&&) = delete;
    Cfcheckpt& operator=(const Cfcheckpt&) = delete;
    Cfcheckpt& operator=(Cfcheckpt&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
