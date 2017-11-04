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

#include "opentxs/cash/Mint.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/server/MessageProcessor.hpp"
#include "opentxs/server/Server.hpp"

#define PUBLIC_SERIES ".PUBLIC"

#define OT_METHOD "opentxs::Server::"

namespace opentxs::api
{
Server::Server(
    const std::map<std::string, std::string>& args,
    std::atomic<bool>& shutdown)
    : args_(args)
    , shutdown_(shutdown)
    , server_p_(new server::Server)
    , server_(*server_p_)
    , message_processor_p_(new server::MessageProcessor(server_, shutdown_))
    , message_processor_(*message_processor_p_)
    , mint_lock_()
    , mint_update_lock_()
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

std::shared_ptr<Mint> Server::GetPrivateMint(
    const Identifier& unitID,
    std::uint32_t index) const
{
    Lock lock(mint_lock_);
    const std::string id{String(unitID).Get()};
    const std::string seriesID = std::string(".") + std::to_string(index);
    auto currency = mints_.find(id);

    if (mints_.end() == currency) {

        return load_private_mint(lock, id, seriesID);
    }

    auto& seriesMap = currency->second;
    // Modifying the private version may invalid the public version
    seriesMap.erase(PUBLIC_SERIES);
    auto series = seriesMap.find(seriesID);

    if (seriesMap.end() == series) {

        return load_private_mint(lock, id, seriesID);
    }

    return series->second;
}

std::shared_ptr<const Mint> Server::GetPublicMint(
    const Identifier& unitID) const
{
    Lock lock(mint_lock_);
    const std::string id{String(unitID).Get()};
    const std::string seriesID{PUBLIC_SERIES};
    auto currency = mints_.find(id);

    if (mints_.end() == currency) {

        return load_public_mint(lock, id, seriesID);
    }

    auto& seriesMap = currency->second;
    auto series = seriesMap.find(seriesID);

    if (seriesMap.end() == series) {

        return load_public_mint(lock, id, seriesID);
    }

    return series->second;
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

std::shared_ptr<Mint> Server::load_private_mint(
    const Lock& lock,
    const std::string& unitID,
    const std::string seriesID) const
{
    OT_ASSERT(verify_lock(lock, mint_lock_));

    std::shared_ptr<Mint> mint(
        Mint::MintFactory(String(ID()), String(NymID()), unitID.c_str()));

    OT_ASSERT(mint);

    return verify_mint(lock, unitID, seriesID, mint);
}

std::shared_ptr<Mint> Server::load_public_mint(
    const Lock& lock,
    const std::string& unitID,
    const std::string seriesID) const
{
    OT_ASSERT(verify_lock(lock, mint_lock_));

    std::shared_ptr<Mint> mint(Mint::MintFactory(String(ID()), unitID.c_str()));

    OT_ASSERT(mint);

    return verify_mint(lock, unitID, seriesID, mint);
}

const Identifier& Server::NymID() const { return server_.GetServerNym().ID(); }

void Server::Start() { message_processor_.Start(); }

bool Server::verify_lock(const Lock& lock, const std::mutex& mutex) const
{
    if (lock.mutex() != &mutex) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect mutex." << std::endl;

        return false;
    }

    if (false == lock.owns_lock()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock not owned." << std::endl;

        return false;
    }

    return true;
}

std::shared_ptr<Mint> Server::verify_mint(
    const Lock& lock,
    const std::string& unitID,
    const std::string seriesID,
    std::shared_ptr<Mint>& mint) const
{
    OT_ASSERT(verify_lock(lock, mint_lock_));

    if (false == mint->LoadMint(seriesID.c_str())) {

        return {};
    }

    if (false == mint->VerifyMint(server_.GetServerNym())) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid mint for " << unitID
              << std::endl;

        return {};
    }

    auto& output = mints_[unitID][seriesID];

    OT_ASSERT(false == bool(output));

    output = mint;

    OT_ASSERT(true == bool(output));

    return output;
}

Server::~Server() {}
}  // namespace opentxs::api
