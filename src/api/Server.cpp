// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Wallet.hpp"
#if OT_CASH
#include "opentxs/cash/Mint.hpp"
#endif  // OT_CASH
#include <opentxs/core/util/OTDataFolder.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/util/OTPaths.hpp>
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include <opentxs/core/OTStorage.hpp>
#include "opentxs/core/String.hpp"

#include "server/MessageProcessor.hpp"
#include "server/Server.hpp"
#include "server/ServerSettings.hpp"

#include <chrono>
#include <ctime>

#include "Server.hpp"

#if OT_CASH
#define SERIES_DIVIDER "."
#define PUBLIC_SERIES ".PUBLIC"
#define MAX_MINT_SERIES 10000
#define MINT_EXPIRE_MONTHS 6
#define MINT_VALID_MONTHS 12
#define MINT_GENERATE_DAYS 7
#endif  // OT_CASH

#define OT_METHOD "opentxs::api::implementation::Server::"

namespace opentxs::api::implementation
{
Server::Server(
    const ArgList& args,
    const opentxs::api::Crypto& crypto,
    const opentxs::api::Settings& config,
    const opentxs::api::storage::Storage& storage,
    const opentxs::api::client::Wallet& wallet,
    const Flag& running,
    const opentxs::network::zeromq::Context& context)
    : args_(args)
    , config_(config)
    , crypto_(crypto)
    , storage_(storage)
    , wallet_(wallet)
    , running_(running)
    , zmq_context_(context)
    , server_p_(new server::Server(crypto_, config_, *this, storage_, wallet_))
    , server_(*server_p_)
    , message_processor_p_(
          new server::MessageProcessor(server_, context, running_))
    , message_processor_(*message_processor_p_)
#if OT_CASH
    , mint_thread_(nullptr)
    , mint_lock_()
    , mint_update_lock_()
    , mint_scan_lock_()
    , mints_()
    , mints_to_check_()
#endif  // OT_CASH
{
    OT_ASSERT(server_p_);
    OT_ASSERT(message_processor_p_);

#if OT_CASH
    mint_thread_.reset(new std::thread(&Server::mint, this));
#endif  // OT_CASH
}

void Server::Cleanup()
{
    otErr << OT_METHOD << __FUNCTION__ << ": Shutting down and cleaning up."
          << std::endl;

    message_processor_.cleanup();
}

#if OT_CASH
void Server::generate_mint(
    const std::string& serverID,
    const std::string& unitID,
    const std::uint32_t series) const
{
    auto mint = GetPrivateMint(Identifier::Factory(unitID), series);

    if (mint) {
        otErr << OT_METHOD << __FUNCTION__ << ": Mint already exists."
              << std::endl;

        return;
    }

    const std::string nymID{NymID().str()};
    const std::string seriesID =
        std::string(SERIES_DIVIDER) + std::to_string(series);
    mint.reset(
        Mint::MintFactory(serverID.c_str(), nymID.c_str(), unitID.c_str()));

    OT_ASSERT(mint)

    const auto& nym = server_.GetServerNym();
    const std::time_t now = std::time(nullptr);
    const std::chrono::seconds expireInterval(
        std::chrono::hours(MINT_EXPIRE_MONTHS * 30 * 24));
    const std::chrono::seconds validInterval(
        std::chrono::hours(MINT_VALID_MONTHS * 30 * 24));
    const std::time_t expires = now + expireInterval.count();
    const std::time_t validTo = now + validInterval.count();

    if (false == verify_mint_directory(serverID)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to create mint directory." << std::endl;

        return;
    }

    mint->GenerateNewMint(
        wallet_,
        series,
        now,
        validTo,
        expires,
        Identifier::Factory(unitID),
        Identifier::Factory(serverID),
        nym,
        1,
        5,
        10,
        25,
        100,
        500,
        1000,
        2000,
        10000,
        100000);

    Lock mintLock(mint_lock_);

    if (mints_.end() != mints_.find(unitID)) {
        mints_.at(unitID).erase(PUBLIC_SERIES);
    }

    mint->SetSavePrivateKeys();
    mint->SignContract(nym);
    mint->SaveContract();
    mint->SaveMint();
    mint->SaveMint(seriesID.c_str());
    mint->ReleaseSignatures();
    mint->SignContract(nym);
    mint->SaveContract();
    mint->SaveMint(PUBLIC_SERIES);
}

const std::string Server::get_arg(const std::string& argName) const
{
    auto argIt = args_.find(argName);
    if (args_.end() != argIt) {
        const auto& argItems = argIt->second;
        OT_ASSERT(2 > argItems.size());
        OT_ASSERT(0 < argItems.size());
        return *argItems.cbegin();
    }
    return {};
}

const std::string Server::GetCommandPort() const
{
    return get_arg(OPENTXS_ARG_COMMANDPORT);
}

const std::string Server::GetDefaultBindIP() const
{
    return get_arg(OPENTXS_ARG_BINDIP);
}

const std::string Server::GetEEP() const { return get_arg(OPENTXS_ARG_EEP); }

const std::string Server::GetExternalIP() const
{
    return get_arg(OPENTXS_ARG_EXTERNALIP);
}

const std::string Server::GetListenCommand() const
{
    return get_arg(OPENTXS_ARG_LISTENCOMMAND);
}

const std::string Server::GetListenNotify() const
{
    return get_arg(OPENTXS_ARG_LISTENNOTIFY);
}

const std::string Server::GetOnion() const
{
    return get_arg(OPENTXS_ARG_ONION);
}

std::shared_ptr<Mint> Server::GetPrivateMint(
    const Identifier& unitID,
    std::uint32_t index) const
{
    Lock lock(mint_lock_);
    const std::string id{unitID.str()};
    const std::string seriesID =
        std::string(SERIES_DIVIDER) + std::to_string(index);
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
    const std::string id{unitID.str()};
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
#endif  // OT_CASH

const std::string Server::GetUserName() const
{
    return get_arg(OPENTXS_ARG_NAME);
}

const std::string Server::GetUserTerms() const
{
    return get_arg(OPENTXS_ARG_TERMS);
}

const Identifier& Server::ID() const { return server_.GetServerID(); }

void Server::Init() {}

#if OT_CASH
std::int32_t Server::last_generated_series(
    const std::string& serverID,
    const std::string& unitID) const
{
    std::uint32_t output{0};

    for (output = 0; output < MAX_MINT_SERIES; ++output) {
        const std::string filename =
            unitID + SERIES_DIVIDER + std::to_string(output);
        const auto exists = OTDB::Exists(
            OTFolders::Mint().Get(), serverID.c_str(), filename.c_str());

        if (false == exists) { return output - 1; }
    }

    return -1;
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

void Server::mint() const
{
    Lock updateLock(mint_update_lock_, std::defer_lock);

    while (server_.GetServerID().empty()) {
        Log::Sleep(std::chrono::milliseconds(50));
    }

    const std::string serverID{server_.GetServerID().str()};

    OT_ASSERT(false == serverID.empty());

    while (running_) {
        Log::Sleep(std::chrono::milliseconds(250));

        if (false == server::ServerSettings::__cmd_get_mint) { continue; }

        std::string unitID{""};
        updateLock.lock();

        if (0 < mints_to_check_.size()) {
            unitID = mints_to_check_.back();
            mints_to_check_.pop_back();
        }

        updateLock.unlock();

        if (unitID.empty()) { continue; }

        const auto last = last_generated_series(serverID, unitID);
        const auto next = last + 1;

        if (0 > last) {
            generate_mint(serverID, unitID, 0);

            continue;
        }

        auto mint = GetPrivateMint(Identifier::Factory(unitID), last);

        if (false == bool(mint)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to load existing series." << std::endl;

            continue;
        }

        const auto now = std::time(nullptr);
        const std::time_t expires = mint->GetExpiration();
        const std::chrono::seconds limit(
            std::chrono::hours(24 * MINT_GENERATE_DAYS));
        const bool generate = ((now + limit.count()) > expires);

        if (generate) {
            generate_mint(serverID, unitID, next);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Existing mint file for "
                  << unitID << " is still valid." << std::endl;
        }
    }
}
#endif  // OT_CASH

const Identifier& Server::NymID() const { return server_.GetServerNym().ID(); }

#if OT_CASH
void Server::ScanMints() const
{
    Lock scanLock(mint_scan_lock_);
    Lock updateLock(mint_update_lock_, std::defer_lock);
    const auto units = wallet_.UnitDefinitionList();

    for (const auto& it : units) {
        const auto& id = it.first;
        updateLock.lock();
        mints_to_check_.push_front(id);
        updateLock.unlock();
    }
}
#endif  // OT_CASH

void Server::Start()
{
    server_.Init();
    server_.ActivateCron();
    std::string hostname{};
    std::uint32_t port{0};
    const auto connectInfo = server_.GetConnectInfo(hostname, port);

    OT_ASSERT(connectInfo);

    auto pubkey = Data::Factory();
    auto privateKey = server_.TransportKey(pubkey);

    OT_ASSERT(privateKey);

    message_processor_.init(port, *privateKey);
    message_processor_.Start();
#if OT_CASH
    ScanMints();
#endif  // OT_CASH
}

#if OT_CASH
void Server::UpdateMint(const Identifier& unitID) const
{
    Lock updateLock(mint_update_lock_);
    mints_to_check_.push_front(unitID.str());
}
#endif  // OT_CASH

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

#if OT_CASH
std::shared_ptr<Mint> Server::verify_mint(
    const Lock& lock,
    const std::string& unitID,
    const std::string seriesID,
    std::shared_ptr<Mint>& mint) const
{
    OT_ASSERT(verify_lock(lock, mint_lock_));

    if (false == mint->LoadMint(seriesID.c_str())) {
        UpdateMint(Identifier::Factory(unitID));

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

bool Server::verify_mint_directory(const std::string& serverID) const
{
    bool created{false};
    String serverDir{""};
    String mintDir{""};
    const auto haveMint =
        OTPaths::AppendFolder(mintDir, OTDataFolder::Get(), OTFolders::Mint());
    const auto haveServer =
        OTPaths::AppendFolder(serverDir, mintDir, serverID.c_str());

    OT_ASSERT(haveMint)
    OT_ASSERT(haveServer)

    return OTPaths::BuildFolderPath(serverDir, created);
}
#endif  // OT_CASH

Server::~Server()
{
#if OT_CASH
    if (mint_thread_) {
        mint_thread_->join();
        mint_thread_.reset();
    }
#endif  // OT_CASH
}
}  // namespace opentxs::api::implementation
