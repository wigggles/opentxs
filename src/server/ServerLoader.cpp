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

#include "opentxs/server/ServerLoader.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/util/OTDataFolder.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/server/OTServer.hpp"

namespace opentxs
{
OTServer* ServerLoader::server_ = nullptr;

ServerLoader::ServerLoader(std::map<std::string, std::string>& args)
{
    // This is optional! (I, of course, am using it in this test app...)
#if defined(OT_SIGNAL_HANDLING)
    Log::SetupSignalHandler();
#endif

    // I instantiate this here (instead of globally) so that I am assured that
    // any globals and other setup is already done before we instantiate the
    // server object itself.
    OT_ASSERT_MSG(nullptr == server_,
                    "server main(): ASSERT: nullptr == server_.");
    server_ = new OTServer;

    OT_ASSERT_MSG(
        nullptr != server_,
        "server main(): ASSERT: Unable to instantiate OT server.\n");

    {
        bool setupPathsSuccess = false;
        if (!OTDataFolder::Init(SERVER_CONFIG_KEY)) {
            OT_FAIL;
        }
        else {
            setupPathsSuccess = true;
        }
        OT_ASSERT_MSG(setupPathsSuccess,
                        "main(): Assert failed: Failed to set OT Path");

        if (!OTDataFolder::IsInitialized()) {
            OT_FAIL;
        }
    }

    OT::Factory(true);

    // OTServer::Init loads up server's nym so it can decrypt messages sent
    // in envelopes. It also does various other initialization work.
    //
    // NOTE: Envelopes prove that ONLY someone who actually had the server
    // contract, and had loaded it into his wallet, could ever connect to
    // the server or communicate with it. And if that person is following
    // the contract, there is only one server he can connect to, and one
    // key he can use to talk to it.
    //
    // Keys, etc are loaded here. Assumes main path is set!
    server_->Init(args);

    // A heartbeat for recurring transactions, such as markets, payment
    // plans, and smart contracts.
    server_->ActivateCron();
}

int ServerLoader::getPort() const
{
    OT_ASSERT(nullptr != server_);

    // We're going to listen on the same port that is listed in our server
    // contract. The hostname of this server, according to its own contract.
    std::string hostname;
    uint32_t port = 0;
    bool connectInfo = server_->GetConnectInfo(hostname, port);

    OT_ASSERT_MSG(connectInfo,
                    "server main: Unable to find my own connect "
                    "info (which SHOULD be in my server contract, "
                    "BTW.) Perhaps you failed trying to open that "
                    "contract? Have you tried the test password? "
                    "(\"test\")\n");
    return port;
}

OTServer* ServerLoader::getServer()
{
    return server_;
}

zcert_t* ServerLoader::getTransportKey() const
{
    OT_ASSERT(nullptr != server_);

    return server_->GetTransportKey();
}

ServerLoader::~ServerLoader()
{
    Log::vOutput(0, "\n\n Shutting down and cleaning up.\n");

    if (nullptr != server_) {
        delete server_;
        server_ = nullptr;
    }

    OT::Cleanup();
}
} // namespace opentxs
