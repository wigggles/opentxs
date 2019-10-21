// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Filteradd final : virtual public internal::Filteradd
{
public:
    OTData Element() const noexcept final { return element_; }

    ~Filteradd() final = default;

private:
    friend opentxs::Factory;

    const OTData element_;

    OTData payload() const noexcept final;

    Filteradd(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& element) noexcept;
    Filteradd(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const Data& element) noexcept;
    Filteradd(const Filteradd&) = delete;
    Filteradd(Filteradd&&) = delete;
    Filteradd& operator=(const Filteradd&) = delete;
    Filteradd& operator=(Filteradd&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
