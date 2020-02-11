// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
template <typename InterfaceType>
class Nopayload final : virtual public InterfaceType
{
public:
    ~Nopayload() final = default;

private:
    friend opentxs::Factory;

    OTData payload() const noexcept final { return Data::Factory(); }

    Nopayload(
        const api::internal::Core& api,
        const blockchain::Type network,
        const bitcoin::Command command) noexcept
        : Message(api, network, command)
    {
        Message::init_hash();
    }
    Nopayload(
        const api::internal::Core& api,
        std::unique_ptr<Header> header) noexcept
        : Message(api, std::move(header))
    {
    }
    Nopayload(const Nopayload&) = delete;
    Nopayload(Nopayload&&) = delete;
    Nopayload& operator=(const Nopayload&) = delete;
    Nopayload& operator=(Nopayload&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
