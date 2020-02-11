// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Headers final : virtual internal::Headers
{
public:
    const value_type& at(const std::size_t position) const noexcept(false) final
    {
        return *payload_.at(position);
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

    ~Headers() final = default;

private:
    friend opentxs::Factory;

    const std::vector<std::unique_ptr<value_type>> payload_;

    OTData payload() const noexcept final;

    Headers(
        const api::internal::Core& api,
        const blockchain::Type network,
        std::vector<std::unique_ptr<value_type>>&& headers) noexcept;
    Headers(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        std::vector<std::unique_ptr<value_type>>&& headers) noexcept;
    Headers(const Headers&) = delete;
    Headers(Headers&&) = delete;
    Headers& operator=(const Headers&) = delete;
    Headers& operator=(Headers&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
