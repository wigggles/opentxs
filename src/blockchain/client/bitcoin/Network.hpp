// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::client::bitcoin::implementation
{
class Network final : public client::implementation::Network
{
public:
    std::unique_ptr<block::Header> instantiate_header(
        const network::zeromq::Frame& payload) const noexcept final;

    ~Network() final;

private:
    friend opentxs::Factory;

    using ot_super = client::implementation::Network;

    Network(
        const api::internal::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const Type type,
        const std::string& seednode,
        const std::string& shutdown);
    Network() = delete;
    Network(const Network&) = delete;
    Network(Network&&) = delete;
    Network& operator=(const Network&) = delete;
    Network& operator=(Network&&) = delete;
};
}  // namespace opentxs::blockchain::client::bitcoin::implementation
