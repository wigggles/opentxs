// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Filterload final : virtual public internal::Filterload
{
public:
    virtual OTBloomFilter Filter() const noexcept { return payload_; }

    ~Filterload() final = default;

private:
    friend opentxs::Factory;

    const OTBloomFilter payload_;

    OTData payload() const noexcept final { return payload_->Serialize(); }

    Filterload(
        const api::internal::Core& api,
        const blockchain::Type network,
        const blockchain::BloomFilter& filter) noexcept;
    Filterload(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const blockchain::BloomFilter& filter) noexcept;
    Filterload(const Filterload&) = delete;
    Filterload(Filterload&&) = delete;
    Filterload& operator=(const Filterload&) = delete;
    Filterload& operator=(Filterload&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
