// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/** \defgroup native Native API */

#ifndef OPENTXS_API_NATIVE_HPP
#define OPENTXS_API_NATIVE_HPP

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

    EXPORT virtual const api::client::Manager& Client(
        const int instance) const = 0;
    EXPORT virtual const api::Settings& Config(
        const std::string& path) const = 0;
    EXPORT virtual const api::Crypto& Crypto() const = 0;
    EXPORT virtual void HandleSignals(
        ShutdownCallback* callback = nullptr) const = 0;
    /** Throws std::out_of_range if the specified server does not exist. */
    EXPORT virtual const api::server::Manager& Server(
        const int instance) const = 0;
    /** Start up a new client
     *
     *  If the specified instance exists, it will be returned.
     *
     *  Otherwise the next instance will be created
     */
    EXPORT virtual const api::client::Manager& StartClient(
        const ArgList& args,
        const int instance) const = 0;
    /** Start up a new server
     *
     *  If the specified instance exists, it will be returned.
     *
     *  Otherwise the next instance will be created
     */
    EXPORT virtual const api::server::Manager& StartServer(
        const ArgList& args,
        const int instance,
        const bool inproc = false) const = 0;

    EXPORT virtual ~Native() = default;

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
#endif
