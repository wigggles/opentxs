// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::rpc::implementation
{
class RPC final : virtual public rpc::internal::RPC, Lockable
{
public:
    proto::RPCResponse Process(const proto::RPCCommand& command) const override;

    ~RPC() = default;

private:
    friend opentxs::Factory;

    using Args = const ::google::protobuf::RepeatedPtrField<
        ::opentxs::proto::APIArgument>;

    const api::Native& ot_;

    static ArgList get_args(const Args& serialized);
    static proto::RPCResponse init(const proto::RPCCommand& command);
    static proto::RPCResponse invalid_command(const proto::RPCCommand& command);

    proto::RPCResponse start_client(const proto::RPCCommand& command) const;
    proto::RPCResponse start_server(const proto::RPCCommand& command) const;

    RPC(const api::Native& native);
    RPC() = delete;
    RPC(const RPC&) = delete;
    RPC(RPC&&) = delete;
    RPC& operator=(const RPC&) = delete;
    RPC& operator=(RPC&&) = delete;
};
}  // namespace opentxs::rpc::implementation
