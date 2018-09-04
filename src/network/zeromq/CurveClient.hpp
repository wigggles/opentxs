// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/Types.hpp"

#include "opentxs/network/zeromq/CurveClient.hpp"

#include <mutex>

namespace opentxs::network::zeromq::implementation
{
class CurveClient : virtual public zeromq::CurveClient
{
public:
    bool SetServerPubkey(const ServerContract& contract) const override;
    bool SetServerPubkey(const Data& key) const override;

protected:
    bool set_public_key(const ServerContract& contract) const;
    bool set_public_key(const Data& key) const;

    CurveClient(std::mutex& lock, void* socket);
    ~CurveClient();

private:
    std::mutex& client_curve_lock_;
    // Not owned by this class
    void* client_curve_socket_{nullptr};

    bool set_local_keys() const;
    bool set_remote_key(const Data& key) const;

    CurveClient() = delete;
    CurveClient(const CurveClient&) = delete;
    CurveClient(CurveClient&&) = delete;
    CurveClient& operator=(const CurveClient&) = delete;
    CurveClient& operator=(CurveClient&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
