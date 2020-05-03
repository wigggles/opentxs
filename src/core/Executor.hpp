// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <functional>
#include <future>

#include "core/StateMachine.hpp"
#include "internal/api/Api.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"

namespace opentxs
{
template <typename Enum>
auto MakeWork(const api::Core& api, const Enum type) noexcept -> OTZMQMessage
{
    auto output = api.ZeroMQ().Message(type);
    output->AddFrame();

    return output;
}

template <typename Child>
class Executor : public internal::StateMachine
{
public:
    template <typename Enum>
    auto MakeWork(const Enum type) const noexcept -> OTZMQMessage
    {
        return opentxs::MakeWork(api_, type);
    }

protected:
    using Callback = internal::StateMachine::Callback;

    const api::internal::Core& api_;
    OTFlag running_;
    std::promise<bool> state_machine_;
    std::promise<void> shutdown_promise_;
    OTZMQPipeline pipeline_;

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

    auto stop_executor() noexcept -> std::shared_future<void>
    {
        pipeline_->Close();

        if (running_.get()) { downcast().shutdown(shutdown_promise_); }

        Stop().get();

        return shutdown_;
    }

    Executor(const api::internal::Core& api, const Callback cb = {}) noexcept
        : StateMachine(
              cb ? cb : [this]() -> bool { return default_state_machine(); })
        , api_(api)
        , running_(Flag::Factory(true))
        , state_machine_()
        , shutdown_promise_()
        , pipeline_(api.Factory().Pipeline(
              [this](auto& in) { downcast().pipeline(in); }))
        , shutdown_(shutdown_promise_.get_future())
    {
    }

    ~Executor() override { stop_executor().get(); }

private:
    std::shared_future<void> shutdown_;

    inline auto downcast() noexcept -> Child&
    {
        return static_cast<Child&>(*this);
    }
    auto default_state_machine() noexcept -> bool
    {
        if (false == running_.get()) { return false; }

        state_machine_ = {};
        auto future = state_machine_.get_future();
        auto work =
            api_.ZeroMQ().Message(std::uint8_t{OT_ZMQ_STATE_MACHINE_SIGNAL});
        work->AddFrame();
        pipeline_->Push(work);

        while (running_.get()) {
            if (std::future_status::ready ==
                future.wait_for(std::chrono::milliseconds(1))) {
                return future.get();
            }
        }

        return false;
    }
};
}  // namespace opentxs
