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

#include "opentxs/stdafx.hpp"

#include "Native.hpp"

#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Activity.hpp"
#include "opentxs/api/Blockchain.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OT_ME.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/core/crypto/Bip39.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTDataFolder.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/storage/StorageConfig.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Signals.hpp"
#include "opentxs/OT.hpp"

#include "api/client/Wallet.hpp"
#include "api/crypto/Crypto.hpp"
#include "api/network/Dht.hpp"
#include "api/network/ZMQ.hpp"
#include "api/storage/Storage.hpp"
#include "api/Api.hpp"
#include "api/Server.hpp"
#include "network/DhtConfig.hpp"
#include "network/OpenDHT.hpp"

#include <atomic>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#define CLIENT_CONFIG_KEY "client"
#define SERVER_CONFIG_KEY "server"
#define STORAGE_CONFIG_KEY "storage"

#define OT_METHOD "opentxs::api::implementation::Native::"

namespace opentxs::api::implementation
{
Native::Native(
    Flag& running,
    const ArgList& args,
    const bool recover,
    const bool serverMode,
    const std::chrono::seconds gcInterval)
    : running_(running)
    , recover_(recover)
    , server_mode_(serverMode)
    , nym_publish_interval_(std::numeric_limits<std::int64_t>::max())
    , nym_refresh_interval_(std::numeric_limits<std::int64_t>::max())
    , server_publish_interval_(std::numeric_limits<std::int64_t>::max())
    , server_refresh_interval_(std::numeric_limits<std::int64_t>::max())
    , unit_publish_interval_(std::numeric_limits<std::int64_t>::max())
    , unit_refresh_interval_(std::numeric_limits<std::int64_t>::max())
    , gc_interval_(gcInterval)
    , config_lock_()
    , task_list_lock_()
    , signal_handler_lock_()
    , periodic_task_list()
    , activity_(nullptr)
    , api_(nullptr)
    , blockchain_(nullptr)
    , config_()
    , contacts_(nullptr)
    , crypto_(nullptr)
    , dht_(nullptr)
    , identity_(nullptr)
    , storage_(nullptr)
    , wallet_(nullptr)
    , zeromq_(nullptr)
    , periodic_(nullptr)
    , storage_encryption_key_(nullptr)
    , server_(nullptr)
    , zmq_context_(opentxs::network::zeromq::Context::Factory())
    , signal_handler_(nullptr)
    , server_args_(args)
{
    for (const auto & [ key, arg ] : args) {
        if (key == OPENTXS_ARG_WORDS) {
            OT_ASSERT(2 > arg.size());
            OT_ASSERT(0 < arg.size());
            const auto& word = *arg.cbegin();
            word_list_.setPassword(word.c_str(), word.size());
        } else if (key == OPENTXS_ARG_PASSPHRASE) {
            OT_ASSERT(2 > arg.size());
            OT_ASSERT(0 < arg.size());
            const auto& passphrase = *arg.cbegin();
            passphrase_.setPassword(passphrase.c_str(), passphrase.size());
        } else if (key == OPENTXS_ARG_STORAGE_PLUGIN) {
            OT_ASSERT(2 > arg.size());
            OT_ASSERT(0 < arg.size());
            const auto& storagePlugin = *arg.cbegin();
            primary_storage_plugin_ = storagePlugin;
        } else if (key == OPENTXS_ARG_BACKUP_DIRECTORY) {
            OT_ASSERT(2 > arg.size());
            OT_ASSERT(0 < arg.size());
            const auto& backupDirectory = *arg.cbegin();
            archive_directory_ = backupDirectory;
        } else if (key == OPENTXS_ARG_ENCRYPTED_DIRECTORY) {
            OT_ASSERT(2 > arg.size());
            OT_ASSERT(0 < arg.size());
            const auto& encryptedDirectory = *arg.cbegin();
            encrypted_directory_ = encryptedDirectory;
        }
    }
}

const api::Activity& Native::Activity() const
{
    OT_ASSERT(activity_)

    return *activity_;
}

const api::Api& Native::API() const
{
    if (server_mode_) {
        OT_FAIL;
    }

    OT_ASSERT(api_);

    return *api_;
}

const api::Blockchain& Native::Blockchain() const
{
    OT_ASSERT(blockchain_)

    return *blockchain_;
}

const api::Settings& Native::Config(const std::string& path) const
{
    std::unique_lock<std::mutex> lock(config_lock_);
    auto& config = config_[path];

    if (!config) {
        config.reset(new api::Settings(String(path)));
    }

    OT_ASSERT(config);

    lock.unlock();

    return *config;
}

const api::ContactManager& Native::Contact() const
{
    OT_ASSERT(contacts_)

    return *contacts_;
}

const api::Crypto& Native::Crypto() const
{
    OT_ASSERT(crypto_)

    return *crypto_;
}

const api::storage::Storage& Native::DB() const
{
    OT_ASSERT(storage_)

    return *storage_;
}

const api::network::Dht& Native::DHT() const
{
    OT_ASSERT(dht_)

    return *dht_;
}

String Native::get_primary_storage_plugin(
    const StorageConfig& config,
    bool& migrate,
    String& previous) const
{
    const String hardcoded = config.primary_plugin_.c_str();
    const String commandLine = primary_storage_plugin_.c_str();
    String configured{""};
    bool notUsed{false};
    Config().Check_str(
        STORAGE_CONFIG_KEY,
        STORAGE_CONFIG_PRIMARY_PLUGIN_KEY,
        configured,
        notUsed);
    const auto haveConfigured = configured.Exists();
    const auto haveCommandline = commandLine.Exists();
    const bool same = (configured == commandLine);
    if (haveCommandline) {
        if (haveConfigured && (false == same)) {
            migrate = true;
            previous = configured;
            otErr << OT_METHOD << __FUNCTION__ << ": Migrating from "
                  << previous << "." << std::endl;
        }

        return commandLine;
    } else {
        if (haveConfigured) {
            otWarn << OT_METHOD << __FUNCTION__ << ": Using config file value."
                   << std::endl;

            return configured;
        } else {
            otWarn << OT_METHOD << __FUNCTION__ << ": Using default value."
                   << std::endl;

            return hardcoded;
        }
    }
}

void Native::HandleSignals() const
{
    Lock lock(signal_handler_lock_);

    if (false == bool(signal_handler_)) {
        signal_handler_.reset(new Signals(running_));
    }
}

const api::Identity& Native::Identity() const
{
    OT_ASSERT(identity_)

    return *identity_;
}

void Native::Init()
{
    Init_Config();
    Init_Log();  // requires Init_Config()
    Init_Crypto();
    Init_Storage();  // requires Init_Config(), Init_Crypto()
    Init_ZMQ();      // requires Init_Config()
    Init_Contracts();
    Init_Dht();         // requires Init_Config()
    Init_Identity();    // requires Init_Contracts()
    Init_Contacts();    // requires Init_Contracts(), Init_Storage()
    Init_Activity();    // requires Init_Storage(), Init_Contacts(),
                        // Init_Contracts()
    Init_Blockchain();  // requires Init_Storage(), Init_Crypto(),
                        // Init_Contracts(), Init_Activity()
    Init_Api();  // requires Init_Config(), Init_Crypto(), Init_Contracts(),
                 // Init_Identity(), Init_Storage(), Init_ZMQ(), Init_Contacts()
                 // Init_Activity()

    if (recover_) {
        recover();
    }

    Init_Server();  // requires Init_Config(), Init_Storage(), Init_Crypto(),
                    // Init_Contracts(), Init_Log(), Init_Contracts()

    start();
}

void Native::Init_Activity()
{
    OT_ASSERT(contacts_);
    OT_ASSERT(wallet_);
    OT_ASSERT(storage_);

    activity_.reset(new api::Activity(*contacts_, *storage_, *wallet_));
}

void Native::Init_Api()
{
    auto& config = config_[""];

    OT_ASSERT(activity_);
    OT_ASSERT(config);
    OT_ASSERT(contacts_);
    OT_ASSERT(wallet_);
    OT_ASSERT(crypto_);
    OT_ASSERT(identity_);

    if (server_mode_) {

        return;
    }

    api_.reset(new api::implementation::Api(
        running_,
        *activity_,
        *config,
        *contacts_,
        *crypto_,
        *identity_,
        *storage_,
        *wallet_,
        *zeromq_));

    OT_ASSERT(api_);
}

void Native::Init_Blockchain()
{
    OT_ASSERT(activity_);
    OT_ASSERT(crypto_);
    OT_ASSERT(storage_);
    OT_ASSERT(wallet_)

    blockchain_.reset(
        new api::Blockchain(*activity_, *crypto_, *storage_, *wallet_));
}

void Native::Init_Config()
{
    bool setupPathsSuccess{false};

    if (server_mode_) {
        setupPathsSuccess = OTDataFolder::Init(SERVER_CONFIG_KEY);
    } else {
        setupPathsSuccess = OTDataFolder::Init(CLIENT_CONFIG_KEY);
    }

    if (false == setupPathsSuccess) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to initialize data folders" << std::endl;

        OT_FAIL;
    }

    OT_ASSERT(OTDataFolder::IsInitialized());

    String strConfigFilePath;
    OTDataFolder::GetConfigFilePath(strConfigFilePath);
    config_[""].reset(new api::Settings(strConfigFilePath));
}

void Native::Init_Contacts()
{
    OT_ASSERT(storage_)
    OT_ASSERT(wallet_)

    contacts_.reset(new api::ContactManager(*storage_, *wallet_));
}

void Native::Init_Contracts()
{
    wallet_.reset(new api::client::implementation::Wallet(*this));
}

void Native::Init_Crypto() { crypto_.reset(new class Crypto(*this)); }

void Native::Init_Dht()
{
    OT_ASSERT(wallet_);

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

    dht_.reset(new api::network::implementation::Dht(config, *wallet_));
}

void Native::Init_Identity()
{
    OT_ASSERT(wallet_);

    identity_.reset(new api::Identity(*wallet_));
}

void Native::Init_Log()
{
    std::string type{};

    if (server_mode_) {
        type = "server";
    } else {
        type = "client";
    }

    if (false == Log::Init(Config(), type.c_str())) {
        abort();
    }
}

void Native::Init_Server()
{
    if (false == server_mode_) {

        return;
    }

    OT_ASSERT(crypto_);
    OT_ASSERT(storage_);
    OT_ASSERT(wallet_);

    server_.reset(new api::implementation::Server(
        server_args_,
        *crypto_,
        Config(),
        *storage_,
        *wallet_,
        running_,
        zmq_context_));

    OT_ASSERT(server_);

    auto server = dynamic_cast<implementation::Server*>(server_.get());

    OT_ASSERT(server);

    server->Init();
}

void Native::Init_Storage()
{
    OT_ASSERT(crypto_);

    Digest hash = std::bind(
        static_cast<bool (api::crypto::Hash::*)(
            const uint32_t, const std::string&, std::string&) const>(
            &api::crypto::Hash::Digest),
        &(Crypto().Hash()),
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3);
    Random random =
        std::bind(&api::crypto::Encode::RandomFilename, &(Crypto().Encode()));
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
    bool migrate{false};
    String old{""};
    String defaultPlugin = get_primary_storage_plugin(config, migrate, old);
    String archiveDirectory{};
    String encryptedDirectory{};

    otWarn << OT_METHOD << __FUNCTION__ << ": Using " << defaultPlugin
           << " as primary storage plugin." << std::endl;

    if (archive_directory_.empty()) {
        archiveDirectory = config.fs_backup_directory_.c_str();
    } else {
        archiveDirectory = archive_directory_.c_str();
    }

    if (encrypted_directory_.empty()) {
        encryptedDirectory = config.fs_encrypted_backup_directory_.c_str();
    } else {
        encryptedDirectory = encrypted_directory_.c_str();
    }

    const bool haveGCInterval = (0 != gc_interval_.count());
    std::int64_t defaultGcInterval{0};
    std::int64_t configGcInterval{0};

    if (haveGCInterval) {
        defaultGcInterval = gc_interval_.count();
    } else {
        defaultGcInterval = config.gc_interval_;
    }

    encrypted_directory_ = encryptedDirectory.Get();

    Config().CheckSet_bool(
        STORAGE_CONFIG_KEY,
        "auto_publish_nyms",
        config.auto_publish_nyms_,
        config.auto_publish_nyms_,
        notUsed);
    Config().CheckSet_bool(
        STORAGE_CONFIG_KEY,
        "auto_publish_servers_",
        config.auto_publish_servers_,
        config.auto_publish_servers_,
        notUsed);
    Config().CheckSet_bool(
        STORAGE_CONFIG_KEY,
        "auto_publish_units_",
        config.auto_publish_units_,
        config.auto_publish_units_,
        notUsed);
    Config().CheckSet_long(
        STORAGE_CONFIG_KEY,
        "gc_interval",
        defaultGcInterval,
        configGcInterval,
        notUsed);
    Config().CheckSet_str(
        STORAGE_CONFIG_KEY,
        "path",
        String(config.path_),
        config.path_,
        notUsed);
#if OT_STORAGE_FS
    Config().CheckSet_str(
        STORAGE_CONFIG_KEY,
        "fs_primary",
        String(config.fs_primary_bucket_),
        config.fs_primary_bucket_,
        notUsed);
    Config().CheckSet_str(
        STORAGE_CONFIG_KEY,
        "fs_secondary",
        String(config.fs_secondary_bucket_),
        config.fs_secondary_bucket_,
        notUsed);
    Config().CheckSet_str(
        STORAGE_CONFIG_KEY,
        "fs_root_file",
        String(config.fs_root_file_),
        config.fs_root_file_,
        notUsed);
    Config().CheckSet_str(
        STORAGE_CONFIG_KEY,
        STORAGE_CONFIG_FS_BACKUP_DIRECTORY_KEY,
        archiveDirectory,
        config.fs_backup_directory_,
        notUsed);
    archiveDirectory = String(config.fs_backup_directory_.c_str());
    Config().CheckSet_str(
        STORAGE_CONFIG_KEY,
        STORAGE_CONFIG_FS_ENCRYPTED_BACKUP_DIRECTORY_KEY,
        encryptedDirectory,
        config.fs_encrypted_backup_directory_,
        notUsed);
    encryptedDirectory = String(config.fs_encrypted_backup_directory_.c_str());
#endif
#if OT_STORAGE_SQLITE
    Config().CheckSet_str(
        STORAGE_CONFIG_KEY,
        "sqlite3_primary",
        String(config.sqlite3_primary_bucket_),
        config.sqlite3_primary_bucket_,
        notUsed);
    Config().CheckSet_str(
        STORAGE_CONFIG_KEY,
        "sqlite3_secondary",
        String(config.sqlite3_secondary_bucket_),
        config.sqlite3_secondary_bucket_,
        notUsed);
    Config().CheckSet_str(
        STORAGE_CONFIG_KEY,
        "sqlite3_control",
        String(config.sqlite3_control_table_),
        config.sqlite3_control_table_,
        notUsed);
    Config().CheckSet_str(
        STORAGE_CONFIG_KEY,
        "sqlite3_root_key",
        String(config.sqlite3_root_key_),
        config.sqlite3_root_key_,
        notUsed);
    Config().CheckSet_str(
        STORAGE_CONFIG_KEY,
        "sqlite3_db_file",
        String(config.sqlite3_db_file_),
        config.sqlite3_db_file_,
        notUsed);
#endif

    if (haveGCInterval) {
        config.gc_interval_ = defaultGcInterval;
        Config().Set_long(
            STORAGE_CONFIG_KEY, "gc_interval", defaultGcInterval, notUsed);
    } else {
        config.gc_interval_ = configGcInterval;
    }

    if (dht_) {
        config.dht_callback_ = std::bind(
            static_cast<void (api::network::Dht::*)(
                const std::string&, const std::string&) const>(
                &api::network::Dht::Insert),
            dht_.get(),
            std::placeholders::_1,
            std::placeholders::_2);
    }

    OT_ASSERT(crypto_);

    storage_.reset(new api::storage::implementation::Storage(
        running_, config, defaultPlugin, migrate, old, hash, random));
    Config().Set_str(
        STORAGE_CONFIG_KEY,
        STORAGE_CONFIG_PRIMARY_PLUGIN_KEY,
        defaultPlugin,
        notUsed);
    Config().Save();
}

void Native::Init_StorageBackup()
{
    OT_ASSERT(storage_);

    auto storage =
        dynamic_cast<api::storage::implementation::Storage*>(storage_.get());

    OT_ASSERT(nullptr != storage);

    storage->InitBackup();

    if (storage_encryption_key_) {
        storage->InitEncryptedBackup(storage_encryption_key_);
    }

    storage->start();
}

void Native::Init_Periodic()
{
    OT_ASSERT(storage_);

    auto storage = storage_.get();
    const auto now = std::chrono::seconds(std::time(nullptr));

    Schedule(
        std::chrono::seconds(nym_publish_interval_),
        [storage]() -> void {
            NymLambda nymLambda(
                [](const serializedCredentialIndex& nym) -> void {
                    OT::App().DHT().Insert(nym);
                });
            storage->MapPublicNyms(nymLambda);
        },
        now);

    Schedule(
        std::chrono::seconds(nym_refresh_interval_),
        [storage]() -> void {
            NymLambda nymLambda(
                [](const serializedCredentialIndex& nym) -> void {
                    OT::App().DHT().GetPublicNym(nym.nymid());
                });
            storage->MapPublicNyms(nymLambda);
        },
        (now - std::chrono::seconds(nym_refresh_interval_) / 2));

    Schedule(
        std::chrono::seconds(server_publish_interval_),
        [storage]() -> void {
            ServerLambda serverLambda(
                [](const proto::ServerContract& server) -> void {
                    OT::App().DHT().Insert(server);
                });
            storage->MapServers(serverLambda);
        },
        now);

    Schedule(
        std::chrono::seconds(server_refresh_interval_),
        [storage]() -> void {
            ServerLambda serverLambda(
                [](const proto::ServerContract& server) -> void {
                    OT::App().DHT().GetServerContract(server.id());
                });
            storage->MapServers(serverLambda);
        },
        (now - std::chrono::seconds(server_refresh_interval_) / 2));

    Schedule(
        std::chrono::seconds(unit_publish_interval_),
        [storage]() -> void {
            UnitLambda unitLambda(
                [](const proto::UnitDefinition& unit) -> void {
                    OT::App().DHT().Insert(unit);
                });
            storage->MapUnitDefinitions(unitLambda);
        },
        now);

    Schedule(
        std::chrono::seconds(unit_refresh_interval_),
        [storage]() -> void {
            UnitLambda unitLambda(
                [](const proto::UnitDefinition& unit) -> void {
                    OT::App().DHT().GetUnitDefinition(unit.id());
                });
            storage->MapUnitDefinitions(unitLambda);
        },
        (now - std::chrono::seconds(unit_refresh_interval_) / 2));

    periodic_.reset(new std::thread(&Native::Periodic, this));
}

void Native::Init_ZMQ()
{
    auto& config = config_[""];

    OT_ASSERT(config);

    zeromq_.reset(
        new api::network::implementation::ZMQ(zmq_context_, *config, running_));
}

void Native::Periodic()
{
    while (running_) {
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

        if (running_) {
            Log::Sleep(std::chrono::milliseconds(100));
        }
    }
}

void Native::recover()
{
    OT_ASSERT(api_);
    OT_ASSERT(crypto_);
    OT_ASSERT(recover_);
    OT_ASSERT(storage_);
    OT_ASSERT(0 < word_list_.getPasswordSize());

    auto& api = api_->OTAPI();
    const auto fingerprint = api.Wallet_ImportSeed(word_list_, passphrase_);

    if (fingerprint.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to import seed."
              << std::endl;
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Imported seed " << fingerprint
              << std::endl;
    }
}

void Native::Schedule(
    const std::chrono::seconds& interval,
    const PeriodicTask& task,
    const std::chrono::seconds& last) const
{
    // Make sure nobody is iterating while we add to the list
    std::lock_guard<std::mutex> listLock(task_list_lock_);

    periodic_task_list.push_back(
        TaskItem{last.count(), interval.count(), task});
}

const api::Server& Native::Server() const
{
    OT_ASSERT(server_);

    return *server_;
}

bool Native::ServerMode() const { return server_mode_; }

void Native::set_storage_encryption()
{
    OT_ASSERT(api_);
    OT_ASSERT(crypto_);

    const bool loaded = api_->OTAPI().LoadWallet();

    OT_ASSERT(loaded);

    auto wallet = api_->OTAPI().GetWallet(nullptr);

    OT_ASSERT(nullptr != wallet);

    auto seed = crypto_->BIP39().DefaultSeed();

    if (seed.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": No default seed." << std::endl;
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Default seed is: " << seed
              << std::endl;
    }

    storage_encryption_key_ = crypto_->GetStorageKey(seed);

    if (storage_encryption_key_) {
        otWarn << OT_METHOD << __FUNCTION__ << ": Obtained storage key "
               << String(storage_encryption_key_->ID()) << std::endl;
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load storage key "
              << seed << std::endl;
    }

    wallet->SaveWallet();
}

void Native::shutdown()
{
    running_.Off();

    if (periodic_) {
        periodic_->join();
    }

    if (server_) {
        auto server = dynamic_cast<implementation::Server*>(server_.get());

        OT_ASSERT(server);

        server->Cleanup();
    }

    if (api_) {
        auto api = dynamic_cast<implementation::Api*>(api_.get());

        OT_ASSERT(api);

        api->Cleanup();
    }

    server_.reset();
    api_.reset();
    blockchain_.reset();
    activity_.reset();
    identity_.reset();
    dht_.reset();
    contacts_.reset();
    wallet_.reset();
    zeromq_.reset();
    storage_.reset();
    crypto_.reset();
    Log::Cleanup();

    for (auto& config : config_) {
        config.second.reset();
    }

    config_.clear();
}

void Native::start()
{
    OT_ASSERT(activity_);
    OT_ASSERT(contacts_);

    if ((false == server_mode_) && (false == encrypted_directory_.empty())) {
        set_storage_encryption();
    }

    Init_StorageBackup();
    contacts_->start();
    activity_->MigrateLegacyThreads();
    Init_Periodic();

    if (server_mode_) {
        OT_ASSERT(server_);
        auto server = dynamic_cast<implementation::Server*>(server_.get());

        OT_ASSERT(server);

        server->Start();
    }
}

const api::client::Wallet& Native::Wallet() const
{
    OT_ASSERT(wallet_)

    return *wallet_;
}

const api::network::ZMQ& Native::ZMQ() const
{
    OT_ASSERT(zeromq_)

    return *zeromq_;
}
}  // namespace opentxs::api::implementation
