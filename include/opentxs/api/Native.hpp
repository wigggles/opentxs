// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/** \defgroup native Native API */

#ifndef OPENTXS_CORE_API_NATIVE_HPP
#define OPENTXS_CORE_API_NATIVE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Types.hpp"

#include <chrono>
#include <functional>
#include <string>

namespace opentxs
{
namespace api
{
class Native
{
public:
    using ShutdownCallback = std::function<void()>;

    virtual const api::Activity& Activity() const = 0;
    virtual const api::Api& API() const = 0;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    virtual const api::Blockchain& Blockchain() const = 0;
#endif
    virtual const api::Settings& Config(
        const std::string& path = std::string("")) const = 0;
    virtual const api::ContactManager& Contact() const = 0;
    virtual const api::Crypto& Crypto() const = 0;
    virtual const storage::Storage& DB() const = 0;
    virtual const network::Dht& DHT() const = 0;
    virtual void HandleSignals(ShutdownCallback* callback = nullptr) const = 0;
    virtual const api::Identity& Identity() const = 0;
    virtual const api::Legacy& Legacy() const = 0;
    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution. */
    virtual void Schedule(
        const std::chrono::seconds& interval,
        const opentxs::PeriodicTask& task,
        const std::chrono::seconds& last = std::chrono::seconds(0)) const = 0;
    virtual const api::Server& Server() const = 0;
    virtual bool ServerMode() const = 0;
    virtual const client::Wallet& Wallet() const = 0;
    virtual const api::UI& UI() const = 0;
    virtual const network::ZMQ& ZMQ() const = 0;

    virtual ~Native() = default;

protected:
    Native() = default;

private:
    Native(const Native&) = delete;
    Native(Native&&) = delete;
    Native& operator=(const Native&) = delete;
    Native& operator=(Native&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_CORE_API_NATIVE_HPP
