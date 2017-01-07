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

#include "opentxs/core/app/App.hpp"

#include "opentxs/core/app/Api.hpp"
#include "opentxs/core/app/Dht.hpp"
#include "opentxs/core/app/Identity.hpp"
#include "opentxs/core/app/Settings.hpp"
#include "opentxs/core/app/Wallet.hpp"
#include "opentxs/core/crypto/CryptoEncodingEngine.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHashEngine.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTDataFolder.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/network/DhtConfig.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/network/ZMQ.hpp"
#include "opentxs/storage/Storage.hpp"
#include "opentxs/storage/StorageConfig.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/storage/Storage.hpp"

#include <atomic>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#define CLIENT_CONFIG_KEY "client"

namespace opentxs
{

App* App::instance_pointer_ = nullptr;

App::App(const bool serverMode)
    : server_mode_(serverMode)
{
    Init();
}

void App::Factory(const bool serverMode)
{
    OT_ASSERT(nullptr == instance_pointer_);

    instance_pointer_ = new App(serverMode);

    OT_ASSERT(nullptr != instance_pointer_);
}

void App::Init()
{
    shutdown_.store(false);
    Init_Crypto();
    Init_Config();
    Init_Storage(); // requires Init_Config()
    Init_Dht();  // requires Init_Config()
    Init_Periodic();  // requires Init_Dht(), Init_Storage()
    Init_ZMQ(); // requires Init_Config()
    Init_Contracts();
    Init_Identity();
    Init_Api(); // requires Init_Config()
}

void App::Init_Api()
{
    if (!server_mode_) {
        api_.reset(new Api(*config_));
    }
}

void App::Init_Config()
{
    if (!server_mode_) {
        if (!OTDataFolder::Init(CLIENT_CONFIG_KEY)) {
            otErr << __FUNCTION__ << ": Unable to Init data folders";
            OT_FAIL;
        }
    }

    String strConfigFilePath;
    OTDataFolder::GetConfigFilePath(strConfigFilePath);
    config_.reset(new Settings(strConfigFilePath));
}

void App::Init_Contracts() { contract_manager_.reset(new class Wallet); }

void App::Init_Crypto() { crypto_.reset(&CryptoEngine::It()); }

void App::Init_Identity() { identity_.reset(new class Identity); }

void App::Init_Storage()
{
    Digest hash = std::bind(
        static_cast<bool (CryptoHashEngine::*)(
            const uint32_t, const std::string&, std::string&) const>(
            &CryptoHashEngine::Digest),
        &(Crypto().Hash()),
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3);

    Random random =
        std::bind(&CryptoEncodingEngine::RandomFilename, &(Crypto().Encode()));

    std::shared_ptr<OTDB::StorageFS> storage(OTDB::StorageFS::Instantiate());
    std::string root_path = OTFolders::Common().Get();
    std::string path;

    if (0 <= storage->ConstructAndCreatePath(
                 path, OTFolders::Common().Get(), ".temp")) {
        path.erase(path.end() - 5, path.end());
    }

    StorageConfig config;
    config.path_ = path;
    bool notUsed;

    Config().CheckSet_bool(
        "storage",
        "auto_publish_nyms",
        config.auto_publish_nyms_,
        config.auto_publish_nyms_,
        notUsed);
    Config().CheckSet_bool(
        "storage",
        "auto_publish_servers_",
        config.auto_publish_servers_,
        config.auto_publish_servers_,
        notUsed);
    Config().CheckSet_bool(
        "storage",
        "auto_publish_units_",
        config.auto_publish_units_,
        config.auto_publish_units_,
        notUsed);
    Config().CheckSet_long(
        "storage",
        "gc_interval",
        config.gc_interval_,
        config.gc_interval_,
        notUsed);
    Config().CheckSet_str(
        "storage", "path", String(config.path_), config.path_, notUsed);
#ifdef OT_STORAGE_FS
    Config().CheckSet_str(
        "storage",
        "fs_primary",
        String(config.fs_primary_bucket_),
        config.fs_primary_bucket_,
        notUsed);
    Config().CheckSet_str(
        "storage",
        "fs_secondary",
        String(config.fs_secondary_bucket_),
        config.fs_secondary_bucket_,
        notUsed);
    Config().CheckSet_str(
        "storage",
        "fs_root_file",
        String(config.fs_root_file_),
        config.fs_root_file_,
        notUsed);
#endif
#ifdef OT_STORAGE_SQLITE
    Config().CheckSet_str(
        "storage",
        "sqlite3_primary",
        String(config.sqlite3_primary_bucket_),
        config.sqlite3_primary_bucket_,
        notUsed);
    Config().CheckSet_str(
        "storage",
        "sqlite3_secondary",
        String(config.sqlite3_secondary_bucket_),
        config.sqlite3_secondary_bucket_,
        notUsed);
    Config().CheckSet_str(
        "storage",
        "sqlite3_control",
        String(config.sqlite3_control_table_),
        config.sqlite3_control_table_,
        notUsed);
    Config().CheckSet_str(
        "storage",
        "sqlite3_root_key",
        String(config.sqlite3_root_key_),
        config.sqlite3_root_key_,
        notUsed);
    Config().CheckSet_str(
        "storage",
        "sqlite3_db_file",
        String(config.sqlite3_db_file_),
        config.sqlite3_db_file_,
        notUsed);
#endif

    if (dht_) {
        config.dht_callback_ = std::bind(
            static_cast<void (Dht::*)(const std::string&, const std::string&)>(
                &Dht::Insert),
            dht_.get(),
            std::placeholders::_1,
            std::placeholders::_2);
    }

    storage_.reset(&Storage::It(hash, random, config));
}

void App::Init_Dht()
{
    DhtConfig config;
    bool notUsed;
    Config().CheckSet_bool(
        "OpenDHT",
        "enable_dht",
        server_mode_ ? true : false,
        config.enable_dht_,
        notUsed);
    Config().CheckSet_long(
        "OpenDHT",
        "nym_publish_interval",
        config.nym_publish_interval_,
        nym_publish_interval_,
        notUsed);
    Config().CheckSet_long(
        "OpenDHT",
        "nym_refresh_interval",
        config.nym_refresh_interval_,
        nym_refresh_interval_,
        notUsed);
    Config().CheckSet_long(
        "OpenDHT",
        "server_publish_interval",
        config.server_publish_interval_,
        server_publish_interval_,
        notUsed);
    Config().CheckSet_long(
        "OpenDHT",
        "server_refresh_interval",
        config.server_refresh_interval_,
        server_refresh_interval_,
        notUsed);
    Config().CheckSet_long(
        "OpenDHT",
        "unit_publish_interval",
        config.unit_publish_interval_,
        unit_publish_interval_,
        notUsed);
    Config().CheckSet_long(
        "OpenDHT",
        "unit_refresh_interval",
        config.unit_refresh_interval_,
        unit_refresh_interval_,
        notUsed);
    Config().CheckSet_long(
        "OpenDHT",
        "listen_port",
        config.default_port_,
        config.listen_port_,
        notUsed);
    Config().CheckSet_str(
        "OpenDHT",
        "bootstrap_url",
        String(config.bootstrap_url_),
        config.bootstrap_url_,
        notUsed);
    Config().CheckSet_str(
        "OpenDHT",
        "bootstrap_port",
        String(config.bootstrap_port_),
        config.bootstrap_port_,
        notUsed);

    dht_.reset(Dht::It(config));
}

void App::Init_Periodic()
{
    auto storage = storage_.get();
    auto now = std::time(nullptr);

    Schedule(
        nym_publish_interval_,
        [storage]() -> void {
            NymLambda nymLambda(
                [](const serializedCredentialIndex& nym) -> void {
                    App::Me().DHT().Insert(nym);
                });
            storage->MapPublicNyms(nymLambda);
        },
        now);

    Schedule(
        nym_refresh_interval_,
        [storage]() -> void {
            NymLambda nymLambda(
                [](const serializedCredentialIndex& nym) -> void {
                    App::Me().DHT().GetPublicNym(nym.nymid());
                });
            storage->MapPublicNyms(nymLambda);
        },
        (now - nym_refresh_interval_ / 2));

    Schedule(
        server_publish_interval_,
        [storage]() -> void {
            ServerLambda serverLambda(
                [](const proto::ServerContract& server) -> void {
                    App::Me().DHT().Insert(server);
                });
            storage->MapServers(serverLambda);
        },
        now);

    Schedule(
        server_refresh_interval_,
        [storage]() -> void {
            ServerLambda serverLambda(
                [](const proto::ServerContract& server) -> void {
                    App::Me().DHT().GetServerContract(server.id());
                });
            storage->MapServers(serverLambda);
        },
        (now - server_refresh_interval_ / 2));

    Schedule(
        unit_publish_interval_,
        [storage]() -> void {
            UnitLambda unitLambda(
                [](const proto::UnitDefinition& unit) -> void {
                    App::Me().DHT().Insert(unit);
                });
            storage->MapUnitDefinitions(unitLambda);
        },
        now);

    Schedule(
        unit_refresh_interval_,
        [storage]() -> void {
            UnitLambda unitLambda(
                [](const proto::UnitDefinition& unit) -> void {
                    App::Me().DHT().GetUnitDefinition(unit.id());
                });
            storage->MapUnitDefinitions(unitLambda);
        },
        (now - unit_refresh_interval_ / 2));

    std::thread periodic(&App::Periodic, this);
    periodic.detach();
}

void App::Init_ZMQ() {
    zeromq_.reset(new class ZMQ(*config_));
}

void App::Periodic()
{
    while (!shutdown_.load()) {
        std::time_t now = std::time(nullptr);

        // Make sure list is not edited while we iterate
        std::unique_lock<std::mutex> listLock(task_list_lock_);

        for (auto& task : periodic_task_list) {
            if ((now - std::get<0>(task)) > std::get<1>(task)) {
                // set "last performed"
                std::get<0>(task) = now;
                // run the task in an independent thread
                auto taskThread = std::thread(std::get<2>(task));
                taskThread.detach();
            }
        }

        listLock.unlock();

        // This method has its own interval checking. Run here to avoid
        // spawning unnecessary threads.
        if (storage_) {
            storage_->RunGC();
        }

        Log::Sleep(std::chrono::milliseconds(100));
    }
}

const App& App::Me()
{
    OT_ASSERT(nullptr != instance_pointer_);

    return *instance_pointer_;
}

Api& App::API() const
{
    if (server_mode_) { OT_FAIL; }

    OT_ASSERT(api_);

    return *api_;
}

Settings& App::Config() const
{
    OT_ASSERT(config_)

    return *config_;
}

Wallet& App::Contract() const
{
    OT_ASSERT(contract_manager_)

    return *contract_manager_;
}

CryptoEngine& App::Crypto() const
{
    OT_ASSERT(crypto_)

    return *crypto_;
}

Storage& App::DB() const
{
    OT_ASSERT(storage_)

    return *storage_;
}

Dht& App::DHT() const
{
    OT_ASSERT(dht_)

    return *dht_;
}

class Identity& App::Identity() const
{
    OT_ASSERT(identity_)

    return *identity_;
}

class ZMQ& App::ZMQ() const
{
    OT_ASSERT(zeromq_)

    return *zeromq_;
}

void App::Schedule(
    const time64_t& interval,
    const PeriodicTask& task,
    const time64_t& last) const
{
    // Make sure nobody is iterating while we add to the list
    std::lock_guard<std::mutex> listLock(task_list_lock_);

    periodic_task_list.push_back(TaskItem{last, interval, task});
}

void App::Cleanup()
{
    if (nullptr != instance_pointer_) {
        delete instance_pointer_;
        instance_pointer_ = nullptr;
    }
}

App::~App()
{
    shutdown_.store(true);

    api_.reset();
    identity_.reset();
    contract_manager_.reset();
    zeromq_.reset();
    dht_.reset();
    storage_.reset();
    config_.reset();
    crypto_.reset();
}
}  // namespace opentxs
