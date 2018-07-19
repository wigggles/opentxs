// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_CURVECLIENT_IMPLEMENTATION_HPP
#define OPENTXS_NETWORK_ZEROMQ_CURVECLIENT_IMPLEMENTATION_HPP

#include "Internal.hpp"

#include "opentxs/Types.hpp"

#include <mutex>

namespace opentxs::network::zeromq::implementation
{
class CurveClient
{
protected:
    bool set_curve(const ServerContract& contract) const;

    CurveClient(std::mutex& lock, void* socket);
    ~CurveClient();

private:
    std::mutex& curve_lock_;
    // Not owned by this class
    void* curve_socket_{nullptr};

    bool set_local_keys(const Lock& lock) const;
    bool set_remote_key(const Lock& lock, const ServerContract& contract) const;

    CurveClient() = delete;
    CurveClient(const CurveClient&) = delete;
    CurveClient(CurveClient&&) = delete;
    CurveClient& operator=(const CurveClient&) = delete;
    CurveClient& operator=(CurveClient&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_CURVECLIENT_IMPLEMENTATION_HPP
