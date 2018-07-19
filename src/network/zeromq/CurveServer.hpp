// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_CURVESERVER_IMPLEMENTATION_HPP
#define OPENTXS_NETWORK_ZEROMQ_CURVESERVER_IMPLEMENTATION_HPP

#include "Internal.hpp"

#include <mutex>

namespace opentxs::network::zeromq::implementation
{
class CurveServer
{
protected:
    bool set_curve(const OTPassword& key) const;

    CurveServer(std::mutex& lock, void* socket);
    ~CurveServer();

private:
    std::mutex& curve_lock_;
    // Not owned by this class
    void* curve_socket_{nullptr};

    CurveServer() = delete;
    CurveServer(const CurveServer&) = delete;
    CurveServer(CurveServer&&) = delete;
    CurveServer& operator=(const CurveServer&) = delete;
    CurveServer& operator=(CurveServer&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_CURVESERVER_IMPLEMENTATION_HPP
