// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <functional>
#include <future>

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "util/Work.hpp"

namespace opentxs
{
template <typename Enum>
auto MakeWork(const api::Core& api, const Enum type) noexcept -> OTZMQMessage
{
    auto output = api.ZeroMQ().Message(type);
    output->AddFrame();

    return output;
}

template <typename Child, typename API = api::client::Manager>
class Worker
{
public:
    template <typename Enum>
    auto MakeWork(const Enum type) const noexcept -> OTZMQMessage
    {
        return opentxs::MakeWork(api_, type);
    }

protected:
    const API& api_;
    const std::chrono::milliseconds rate_limit_;
    OTFlag running_;
    std::promise<void> shutdown_promise_;
    OTZMQPipeline pipeline_;

    auto repeat(const bool again) const noexcept -> void
    {
        if (again) {
            Sleep(rate_limit_);
            trigger();
        }
    }
    auto trigger() const noexcept -> void
    {
        if (false == running_.get()) { return; }

        auto work =
            api_.ZeroMQ().Message(OTZMQWorkType{OT_ZMQ_STATE_MACHINE_SIGNAL});
        work->AddFrame();
        pipeline_->Push(work);
    }

    auto init_executor(const std::vector<std::string> endpoints = {}) noexcept
        -> void
    {
        auto listen = pipeline_->Start(api_.Endpoints().Shutdown());

        OT_ASSERT(listen);

        for (const auto& endpoint : endpoints) {
            listen = pipeline_->Start(endpoint);

            OT_ASSERT(listen);
        }
    }
    auto stop_worker() noexcept -> std::shared_future<void>
    {
        pipeline_->Close();

        if (running_.get()) { downcast().shutdown(shutdown_promise_); }

        return shutdown_;
    }
    Worker(const API& api, const std::chrono::milliseconds rateLimit) noexcept
        : api_(api)
        , rate_limit_(rateLimit)
        , running_(Flag::Factory(true))
        , shutdown_promise_()
        , pipeline_(api.Factory().Pipeline(
              [this](auto& in) { downcast().pipeline(in); }))
        , shutdown_(shutdown_promise_.get_future())
    {
    }

    ~Worker() { stop_worker().get(); }

private:
    std::shared_future<void> shutdown_;

    inline auto downcast() noexcept -> Child&
    {
        return static_cast<Child&>(*this);
    }
};
}  // namespace opentxs
