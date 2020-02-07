// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Block final : public internal::Block
{
public:
    OTData GetBlock() const noexcept final { return payload_; }
    OTData payload() const noexcept final { return payload_; }

    ~Block() final = default;

private:
    friend opentxs::Factory;

    const OTData payload_;

    Block(
        const api::internal::Core& api,
        const blockchain::Type network,
        const Data& block) noexcept;
    Block(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const Data& block) noexcept;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    Block& operator=(const Block&) = delete;
    Block& operator=(Block&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
