// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/zap/Callback.hpp"

namespace opentxs::network::zeromq::zap::implementation
{
class Callback final : virtual zap::Callback
{
public:
    using Lambda = zap::Callback::ReceiveCallback;

    OTZMQZAPReply Process(const zap::Request& request) const override;
    bool SetDomain(const std::string& domain, const ReceiveCallback& callback)
        const override;
    bool SetPolicy(const Policy policy) const override;

    ~Callback() = default;

private:
    friend zap::Callback;

    const Lambda default_callback_;
    mutable std::map<std::string, Lambda> domains_;
    mutable std::mutex domain_lock_;
    mutable std::atomic<Policy> policy_;

    Callback* clone() const override { return new Callback(); }
    OTZMQZAPReply default_callback(const zap::Request& in) const;
    const Lambda& get_domain(const std::string& domain) const;

    Callback();
    Callback(const Callback&) = delete;
    Callback(Callback&&) = delete;
    Callback& operator=(const Callback&) = delete;
    Callback& operator=(Callback&&) = delete;
};
}  // namespace opentxs::network::zeromq::zap::implementation
