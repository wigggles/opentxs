// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/network/zeromq/CurveServer.hpp"

#include <mutex>

namespace opentxs::network::zeromq::implementation
{
class CurveServer : virtual public zeromq::CurveServer
{
public:
    bool SetDomain(const std::string& domain) const override;
    bool SetPrivateKey(const OTPassword& key) const override;

protected:
    bool set_private_key(const OTPassword& key) const;

    CurveServer(std::mutex& lock, void* socket);
    ~CurveServer();

private:
    std::mutex& server_curve_lock_;
    // Not owned by this class
    void* server_curve_socket_{nullptr};

    CurveServer() = delete;
    CurveServer(const CurveServer&) = delete;
    CurveServer(CurveServer&&) = delete;
    CurveServer& operator=(const CurveServer&) = delete;
    CurveServer& operator=(CurveServer&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
