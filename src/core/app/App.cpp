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

#include <chrono>
#include <functional>
#include <iostream>
#include <string>

#include <opentxs/core/app/App.hpp>

#include <opentxs/core/Log.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/app/Settings.hpp>
#include <opentxs/core/util/OTDataFolder.hpp>
#include <opentxs/core/util/OTFolders.hpp>

namespace opentxs
{

App* App::instance_pointer_ = nullptr;

App::App(const bool serverMode)
    : server_mode_(serverMode)
    , last_nym_publish_(std::time(nullptr))
{
    Init();
}

void App::Init()
{
    Init_Config();
    Init_Crypto();
    Init_Storage();
    Init_Dht();
    Init_Periodic();
}

void App::Init_Config()
{
    String strConfigFilePath;
    OTDataFolder::GetConfigFilePath(strConfigFilePath);
    config_ = new Settings(strConfigFilePath);
}

void App::Init_Crypto()
{
    crypto_ = &CryptoEngine::It();
}

void App::Init_Storage()
{
    Digest hash = std::bind(
        static_cast<bool(CryptoHash::*)(
            const uint32_t,
            const std::string&,
            std::string&)>(&CryptoHash::Digest),
        &(Crypto().Hash()),
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3);

    Random random = std::bind(&CryptoUtil::RandomFilename, &(Crypto().Util()));

    std::shared_ptr<OTDB::StorageFS> storage(OTDB::StorageFS::Instantiate());
    std::string root_path = OTFolders::Common().Get();
    std::string path;

    if (0 <= storage->ConstructAndCreatePath(
            path,
            OTFolders::Common().Get(),
            ".temp")) {
        path.erase(path.end() - 5, path.end());
    }

    StorageConfig config;
    config.path_ = path;
    bool notUsed;

    Config().CheckSet_bool(
        "storage", "auto_publish_nyms",
        config.auto_publish_nyms_, config.auto_publish_nyms_, notUsed);
    Config().CheckSet_long(
        "storage", "gc_interval",
        config.gc_interval_, config.gc_interval_, notUsed);
    Config().CheckSet_str(
        "storage", "path",
        config.path_, config.path_, notUsed);
#ifdef OT_STORAGE_FS
    Config().CheckSet_str(
        "storage", "fs_primary",
        config.fs_primary_bucket_, config.fs_primary_bucket_, notUsed);
    Config().CheckSet_str(
        "storage", "fs_secondary",
        config.fs_secondary_bucket_, config.fs_secondary_bucket_, notUsed);
    Config().CheckSet_str(
        "storage", "fs_root_file",
        config.fs_root_file_, config.fs_root_file_, notUsed);
#endif
#ifdef OT_STORAGE_SQLITE
    Config().CheckSet_str(
        "storage", "sqlite3_primary",
        config.sqlite3_primary_bucket_, config.sqlite3_primary_bucket_, notUsed);
    Config().CheckSet_str(
        "storage", "sqlite3_secondary",
        config.sqlite3_secondary_bucket_, config.sqlite3_secondary_bucket_, notUsed);
    Config().CheckSet_str(
        "storage", "sqlite3_control",
        config.sqlite3_control_table_, config.sqlite3_control_table_, notUsed);
    Config().CheckSet_str(
        "storage", "sqlite3_root_key",
        config.sqlite3_root_key_, config.sqlite3_root_key_, notUsed);
    Config().CheckSet_str(
        "storage", "sqlite3_db_file",
        config.sqlite3_db_file_, config.sqlite3_db_file_, notUsed);
#endif

    storage_ = &Storage::It(
        hash,
        random,
        config);
}

void App::Init_Dht()
{
    DhtConfig config;
    bool notUsed;
    Config().CheckSet_long(
        "OpenDHT", "nym_publish_interval",
        config.nym_publish_interval_, nym_publish_interval_, notUsed);
    Config().CheckSet_long(
        "OpenDHT", "listen_port",
        server_mode_ ? config.default_server_port_ : config.default_client_port_,
        config.listen_port_, notUsed);
    Config().CheckSet_str(
        "OpenDHT", "bootstrap_url",
        config.bootstrap_url_, config.bootstrap_url_, notUsed);
    Config().CheckSet_str(
        "OpenDHT", "bootstrap_port",
        config.bootstrap_port_, config.bootstrap_port_, notUsed);

    dht_ = &Dht::It(config);
}

void App::Init_Periodic()
{
    periodic_thread_ = new std::thread(&App::Periodic, this);
}

void App::Periodic()
{
    for (;;) {
        // Collect garbage, if necessary
        if (nullptr != storage_) {
            storage_->RunGC();
        }

        std::time_t time = std::time(nullptr);

        if ((time - last_nym_publish_) > nym_publish_interval_) {

            if ((nullptr != storage_) && (nullptr != dht_)) {
                last_nym_publish_ = time;
                NymLambda nymLambda([](const serializedCredentialIndex& nym)->
                    void { App::Me().DHT().Insert(nym); });
                storage_->MapPublicNyms(nymLambda);
            }
        }
        Log::Sleep(std::chrono::milliseconds(250));
    }
}

App& App::Me(const bool serverMode)
{
    if (nullptr == instance_pointer_)
    {
        instance_pointer_ = new App(serverMode);
    }

    return *instance_pointer_;
}

Settings& App::Config()
{
    OT_ASSERT(nullptr != config_)

    return *config_;
}

CryptoEngine& App::Crypto()
{
    OT_ASSERT(nullptr != crypto_)

    return *crypto_;
}

Storage& App::DB()
{
    OT_ASSERT(nullptr != storage_)

    return *storage_;
}

Dht& App::DHT()
{
    OT_ASSERT(nullptr != dht_)

    return *dht_;
}

void App::Cleanup()
{
    delete dht_;
    dht_ = nullptr;

    delete storage_;
    storage_ = nullptr;

    delete crypto_;
    crypto_ = nullptr;

    delete config_;
    config_ = nullptr;
}

App::~App()
{
    if ((nullptr != periodic_thread_) && periodic_thread_->joinable()) {
        periodic_thread_->join();
        delete periodic_thread_;
        periodic_thread_ = nullptr;
    }
    Cleanup();
}

} // namespace opentxs
