// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <thread>

#include "opentxs/Types.hpp"

namespace opentxs::internal
{
class StateMachine
{
public:
    using Callback = std::function<bool()>;
    using StopPromise = std::promise<void>;
    using StopFuture = std::shared_future<void>;
    using WaitPromise = std::promise<void>;
    using WaitFuture = std::shared_future<void>;

    /** Process the state machine
     *
     * This command will start a background thread if it is not already running
     * to execute the callback function.
     *
     * The callback function will be executed until one of the three conditions
     * is met:
     *   * The callback function returns false
     *   * The Stop() function is called
     *   * The StateMachine destructor is executed
     *
     * \returns true if the callback thread is executing, false if the state
     * machine is stopping, stopped or destructing
     */
    OPENTXS_EXPORT bool Trigger() const noexcept;

    /** Terminate the state machine
     *
     * This command will cease execution of the callback function if it is
     * running and prevent further execution of the callback function
     *
     *
     * \returns a future whose value will be set when the state machine is
     * in a stopped state
     */
    OPENTXS_EXPORT StopFuture Stop() const noexcept;

    /** Detect a state machine idle condition
     *
     * \returns a future whose value will be set the next time the state machine
     * is not executing the callback function
     */
    OPENTXS_EXPORT WaitFuture Wait() const noexcept;

    OPENTXS_EXPORT virtual ~StateMachine()
    {
        if (false == clean_.load()) { Stop(); }

        if (handle_.joinable()) { handle_.join(); }
    }

protected:
    mutable std::mutex decision_lock_;

    OPENTXS_EXPORT const std::atomic<bool>& running() const noexcept
    {
        return running_;
    }
    OPENTXS_EXPORT const std::atomic<bool>& shutdown() const noexcept
    {
        return shutdown_;
    }

    OPENTXS_EXPORT bool trigger(const Lock& decisionLock) const noexcept;

    OPENTXS_EXPORT StateMachine(const Callback callback) noexcept;

private:
    const Callback cb_;
    mutable std::atomic<bool> clean_;
    mutable std::atomic<bool> shutdown_;
    mutable std::atomic<bool> running_;
    mutable std::thread handle_;
    mutable StopPromise stopping_;
    mutable StopFuture stopping_future_;
    mutable WaitPromise waiting_;
    mutable WaitFuture waiting_future_;

    void clean(const Lock& decisionLock) const noexcept;
    void execute() const noexcept;
    WaitFuture make_wait_promise(
        const Lock& decisionLock,
        const bool set = false) const noexcept;

    StateMachine() = delete;
    StateMachine(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;
    StateMachine& operator=(StateMachine&&) = delete;
};
}  // namespace opentxs::internal
