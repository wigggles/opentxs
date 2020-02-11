// Copyright (c) 2010-2020 The Open-Transactions developers
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
    bool Close() const noexcept final;
    const zeromq::Context& Context() const noexcept final
    {
        return sender_->Context();
    }
    bool Start(const std::string& endpoint) const noexcept
    {
        return receiver_->Start(endpoint);
    }

    ~Pipeline();

private:
    friend opentxs::Factory;

    OTZMQPushSocket sender_;
    OTZMQListenCallback callback_;
    OTZMQSubscribeSocket receiver_;

    Pipeline* clone() const noexcept final { return nullptr; }
    bool push(zeromq::Message& data) const noexcept final
    {
        return sender_->Send(data);
    }

    Pipeline(
        const api::internal::Core& api,
        const zeromq::Context& context,
        std::function<void(zeromq::Message&)> callback) noexcept;
    Pipeline() = delete;
    Pipeline(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
