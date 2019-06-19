// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
class Pipeline final : virtual public zeromq::Pipeline
{
public:
    bool Close() const override { return pull_->Close() && push_->Close(); }
    bool Push(const std::string& data) const override
    {
        return push_->Push(data);
    }
    bool Push(const opentxs::Data& data) const override
    {
        return push_->Push(data);
    }
    bool Push(zeromq::Message& data) const override
    {
        return push_->Push(data);
    }

    ~Pipeline();

private:
    friend opentxs::Factory;

    OTZMQListenCallback callback_;
    OTZMQPullSocket pull_;
    OTZMQPushSocket push_;

    Pipeline* clone() const override { return nullptr; }

    Pipeline(
        const api::Core& api,
        const Context& context,
        std::function<void(zeromq::Message&)> callback);
    Pipeline() = delete;
    Pipeline(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
