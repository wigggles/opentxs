// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Blocktxn final : public internal::Blocktxn
{
public:
    OTData BlockTransactions() const noexcept final { return payload_; }
    OTData payload() const noexcept final { return payload_; }

    ~Blocktxn() final = default;

private:
    friend opentxs::Factory;

    const OTData payload_;

    Blocktxn(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& raw_Blocktxn) noexcept;
    Blocktxn(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const Data& raw_Blocktxn) noexcept;
    Blocktxn(const Blocktxn&) = delete;
    Blocktxn(Blocktxn&&) = delete;
    Blocktxn& operator=(const Blocktxn&) = delete;
    Blocktxn& operator=(Blocktxn&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
