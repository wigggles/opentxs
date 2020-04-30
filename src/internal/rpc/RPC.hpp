// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::rpc::internal
{
struct RPC {
    virtual proto::RPCResponse Process(
        const proto::RPCCommand& command) const = 0;

    virtual ~RPC() = default;
};
}  // namespace opentxs::rpc::internal
