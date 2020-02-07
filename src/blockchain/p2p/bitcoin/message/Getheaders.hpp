// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Getheaders final : public internal::Getheaders
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
    block::pHash StopHash() const noexcept final { return stop_; }
    std::size_t size() const noexcept final { return payload_.size(); }
    bitcoin::ProtocolVersionUnsigned Version() const noexcept final
    {
        return version_;
    }

    ~Getheaders() final = default;

private:
    friend opentxs::Factory;

    const bitcoin::ProtocolVersionUnsigned version_;
    const std::vector<block::pHash> payload_;
    const block::pHash stop_;

    OTData payload() const noexcept final;

    Getheaders(
        const api::internal::Core& api,
        const blockchain::Type network,
        const bitcoin::ProtocolVersionUnsigned version,
        std::vector<block::pHash>&& hashes,
        block::pHash&& stop) noexcept;
    Getheaders(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const bitcoin::ProtocolVersionUnsigned version,
        std::vector<block::pHash>&& hashes,
        block::pHash&& stop) noexcept;
    Getheaders(const Getheaders&) = delete;
    Getheaders(Getheaders&&) = delete;
    Getheaders& operator=(const Getheaders&) = delete;
    Getheaders& operator=(Getheaders&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
