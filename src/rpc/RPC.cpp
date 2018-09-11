// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Native.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/Proto.hpp"

#include "internal/rpc/Internal.hpp"

#include "RPC.hpp"

#define OT_METHOD "opentxs::rpc::implementation::RPC::"

namespace opentxs
{
rpc::internal::RPC* Factory::RPC(const api::Native& native)
{
    return new rpc::implementation::RPC(native);
}
}  // namespace opentxs

namespace opentxs::rpc::implementation
{
RPC::RPC(const api::Native& native)
    : Lockable()
    , ot_(native)
{
}

ArgList RPC::get_args(const Args& serialized)
{
    ArgList output{};

    for (const auto& arg : serialized) {
        auto& row = output[arg.key()];

        for (const auto& value : arg.value()) { row.emplace(value); }
    }

    return output;
}

proto::RPCResponse RPC::init(const proto::RPCCommand& command)
{
    proto::RPCResponse output{};
    output.set_version(command.version());
    output.set_cookie(command.cookie());
    output.set_type(command.type());

    return output;
}

proto::RPCResponse RPC::invalid_command(const proto::RPCCommand& command)
{
    auto output = init(command);
    output.set_success(proto::RPCRESPONSE_INVALID);

    return output;
}

proto::RPCResponse RPC::Process(const proto::RPCCommand& command) const
{
    const auto valid = proto::Validate(command, VERBOSE);

    if (false == valid) { return invalid_command(command); }

    switch (command.type()) {
        case proto::RPCCOMMAND_ADDCLIENTSESSION: {
            return start_client(command);
        } break;
        case proto::RPCCOMMAND_ADDSERVERSESSION: {
            return start_server(command);
        } break;
        case proto::RPCCOMMAND_LISTCLIENTSSESSIONS:
        case proto::RPCCOMMAND_LISTSERVERSSESSIONS:
        case proto::RPCCOMMAND_IMPORTHDSEED:
        case proto::RPCCOMMAND_LISTHDSEEDS:
        case proto::RPCCOMMAND_GETHDSEED:
        case proto::RPCCOMMAND_CREATENYM:
        case proto::RPCCOMMAND_LISTNYMS:
        case proto::RPCCOMMAND_GETNYM:
        case proto::RPCCOMMAND_ADDCLAIM:
        case proto::RPCCOMMAND_DELETECLAIM:
        case proto::RPCCOMMAND_IMPORTSERVERCONTRACT:
        case proto::RPCCOMMAND_LISTSERVERCONTRACTS:
        case proto::RPCCOMMAND_REGISTERNYM:
        case proto::RPCCOMMAND_CREATEUNITDEFINITION:
        case proto::RPCCOMMAND_LISTUNITDEFINITIONS:
        case proto::RPCCOMMAND_ISSUEUNITDEFINITION:
        case proto::RPCCOMMAND_CREATEACCOUNT:
        case proto::RPCCOMMAND_LISTACCOUNTS:
        case proto::RPCCOMMAND_GETACCOUNTBALANCE:
        case proto::RPCCOMMAND_GETACCOUNTACTIVITY:
        case proto::RPCCOMMAND_SENDPAYMENT:
        case proto::RPCCOMMAND_MOVEFUNDS:
        case proto::RPCCOMMAND_ADDCONTACT:
        case proto::RPCCOMMAND_LISTCONTACTS:
        case proto::RPCCOMMAND_GETCONTACT:
        case proto::RPCCOMMAND_ADDCONTACTCLAIM:
        case proto::RPCCOMMAND_DELETECONTACTCLAIM:
        case proto::RPCCOMMAND_VERIFYCLAIM:
        case proto::RPCCOMMAND_ACCEPTVERIFICATION:
        case proto::RPCCOMMAND_SENDCONTACTMESSAGE:
        case proto::RPCCOMMAND_GETCONTACTACTIVITY:
        case proto::RPCCOMMAND_ERROR:
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Unsupported command"
                  << std::endl;
        }
    }

    return invalid_command(command);
}

proto::RPCResponse RPC::start_client(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    const auto session{static_cast<std::uint32_t>(ot_.Clients())};
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    try {
        ot_.StartClient(get_args(command.arg()), session);
    } catch (...) {
        success = proto::RPCRESPONSE_INVALID;
    }

    output.set_session(session);
    output.set_success(success);

    return output;
}

proto::RPCResponse RPC::start_server(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    const auto session{static_cast<std::uint32_t>(ot_.Servers())};
    proto::RPCResponseCode success{proto::RPCRESPONSE_SUCCESS};

    try {
        ot_.StartServer(get_args(command.arg()), session);
    } catch (...) {
        success = proto::RPCRESPONSE_INVALID;
    }

    output.set_session(session);
    output.set_success(success);

    return output;
}
}  // namespace opentxs::rpc::implementation
