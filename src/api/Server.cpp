/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/core/stdafx.hpp"

#include "opentxs/api/Server.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/server/MessageProcessor.hpp"
#include "opentxs/server/OTServer.hpp"

#define OT_METHOD "opentxs::Server::"

namespace opentxs
{
Server::Server(
    const std::map<std::string, std::string>& args,
    std::atomic<bool>& shutdown)
    : args_(args)
    , shutdown_(shutdown)
    , server_p_(new OTServer)
    , server_(*server_p_)
    , message_processor_p_(new MessageProcessor(server_, shutdown_))
    , message_processor_(*message_processor_p_)
{
    OT_ASSERT(server_p_);
    OT_ASSERT(message_processor_p_);
}

void Server::Cleanup()
{
    otErr << OT_METHOD << __FUNCTION__ << ": Shutting down and cleaning up."
          << std::endl;

    message_processor_.Cleanup();
}

const Identifier& Server::ID() const { return server_.GetServerID(); }

void Server::Init()
{
    server_.Init(args_);
    server_.ActivateCron();
    std::string hostname{};
    std::uint32_t port{0};
    const auto connectInfo = server_.GetConnectInfo(hostname, port);

    OT_ASSERT(connectInfo);

    message_processor_.Init(port, server_.GetTransportKey());
}

const Identifier& Server::NymID() const { return server_.GetServerNym().ID(); }

void Server::Start() { message_processor_.Start(); }

Server::~Server() {}
}  // namespace opentxs
