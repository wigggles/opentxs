// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/network/zeromq/zap/Callback.cpp"

#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <string>

#include "opentxs/network/zeromq/zap/Callback.hpp"
#include "opentxs/network/zeromq/zap/Reply.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Request;
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::zap::implementation
{
class Callback final : virtual zap::Callback
{
public:
    using Lambda = zap::Callback::ReceiveCallback;

    OTZMQZAPReply Process(const zap::Request& request) const final;
    bool SetDomain(const std::string& domain, const ReceiveCallback& callback)
        const final;
    bool SetPolicy(const Policy policy) const final;

    ~Callback() final = default;

private:
    friend zap::Callback;

    const Lambda default_callback_;
    mutable std::map<std::string, Lambda> domains_;
    mutable std::mutex domain_lock_;
    mutable std::atomic<Policy> policy_;

    Callback* clone() const final { return new Callback(); }
    OTZMQZAPReply default_callback(const zap::Request& in) const;
    const Lambda& get_domain(const std::string& domain) const;

    Callback();
    Callback(const Callback&) = delete;
    Callback(Callback&&) = delete;
    Callback& operator=(const Callback&) = delete;
    Callback& operator=(Callback&&) = delete;
};
}  // namespace opentxs::network::zeromq::zap::implementation
