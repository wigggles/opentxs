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

#include "opentxs/api/OT.hpp"

#include "opentxs/api/Activity.hpp"
#include "opentxs/api/Api.hpp"
#include "opentxs/api/Blockchain.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/api/Dht.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Server.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OT_ME.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OTME_too.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/client/MadeEasy.hpp"
#include "opentxs/core/crypto/Bip39.hpp"
#include "opentxs/core/crypto/CryptoEncodingEngine.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHashEngine.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"
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
#include "opentxs/util/Signals.hpp"

#include <atomic>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#define CLIENT_CONFIG_KEY "client"
#define SERVER_CONFIG_KEY "server"
#define STORAGE_CONFIG_KEY "storage"

#define OT_METHOD "opentxs::OT::"

namespace opentxs
{

OT* OT::instance_pointer_ = nullptr;

OT::OT(
    const bool recover,
    const std::string& words,
    const std::string& passphrase,
    const bool serverMode,
    const std::chrono::seconds gcInterval,
    const std::string& storagePlugin,
    const std::string& backupDirectory,
    const std::string& encryptedDirectory,
    const std::map<std::string, std::string>& serverArgs)
    : recover_(recover)
    , server_mode_(serverMode)
    , nym_publish_interval_(std::numeric_limits<std::int64_t>::max())
    , nym_refresh_interval_(std::numeric_limits<std::int64_t>::max())
    , server_publish_interval_(std::numeric_limits<std::int64_t>::max())
    , server_refresh_interval_(std::numeric_limits<std::int64_t>::max())
    , unit_publish_interval_(std::numeric_limits<std::int64_t>::max())
    , unit_refresh_interval_(std::numeric_limits<std::int64_t>::max())
    , gc_interval_(gcInterval)
    , word_list_(words.c_str(), words.size())
    , passphrase_(passphrase.c_str(), passphrase.size())
    , primary_storage_plugin_(storagePlugin)
    , archive_directory_(backupDirectory)
    , encrypted_directory_(encryptedDirectory)
    , config_lock_()
    , task_list_lock_()
    , signal_handler_lock_()
    , periodic_task_list()
    , shutdown_(false)
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
    , signal_handler_(nullptr)
    , server_args_(serverArgs)
{
}

const OT& OT::App()
{
    OT_ASSERT(nullptr != instance_pointer_);

    return *instance_pointer_;
}

api::Activity& OT::Activity() const
{
    OT_ASSERT(activity_)

    return *activity_;
}

api::Api& OT::API() const
{
    if (server_mode_) {
        OT_FAIL;
    }

    OT_ASSERT(api_);

    return *api_;
}

api::Blockchain& OT::Blockchain() const
{
    OT_ASSERT(blockchain_)

    return *blockchain_;
}

void OT::Cleanup()
{
    if (nullptr != instance_pointer_) {
        instance_pointer_->shutdown();
        delete instance_pointer_;
        instance_pointer_ = nullptr;
    }
}

void OT::ClientFactory(
    const std::chrono::seconds gcInterval,
    const std::string& storagePlugin,
    const std::string& backupDirectory,
    const std::string& encryptedDirectory)
{
    ClientFactory(
        false,
        "",
        "",
        gcInterval,
        storagePlugin,
        backupDirectory,
        encryptedDirectory);
}

void OT::ClientFactory(
    const bool recover,
    const std::string& words,
    const std::string& passphrase,
    const std::chrono::seconds gcInterval,
    const std::string& storagePlugin,
    const std::string& backupDirectory,
    const std::string& encryptedDirectory)
{
    assert(nullptr == instance_pointer_);

    instance_pointer_ = new OT(
        recover,
        words,
        passphrase,
        false,
        gcInterval,
        storagePlugin,
        backupDirectory,
        encryptedDirectory,
        {});

    assert(nullptr != instance_pointer_);

    instance_pointer_->Init();
}

api::Settings& OT::Config(const std::string& path) const
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

api::ContactManager& OT::Contact() const
{
    OT_ASSERT(contacts_)

    return *contacts_;
}

CryptoEngine& OT::Crypto() const
{
    OT_ASSERT(crypto_)

    return *crypto_;
}

api::Storage& OT::DB() const
{
    OT_ASSERT(storage_)

    return *storage_;
}

api::Dht& OT::DHT() const
{
    OT_ASSERT(dht_)

    return *dht_;
}

void OT::HandleSignals() const
{
    Lock lock(signal_handler_lock_);

    if (false == bool(signal_handler_)) {
        signal_handler_.reset(new Signals(shutdown_));
    }
}

api::Identity& OT::Identity() const
{
    OT_ASSERT(identity_)

    return *identity_;
}

void OT::Init()
{
    Init_Config();
    Init_Log();  // requires Init_Config()
    Init_Crypto();
    Init_Storage();  // requires Init_Config(), Init_Crypto()
    Init_Dht();      // requires Init_Config()
    Init_ZMQ();      // requires Init_Config()
    Init_Contracts();
    Init_Identity();
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

    Init_Server();  // requires Init_Config(), Init_Log(), Init_Contracts()

    start();
}

void OT::Init_Activity()
{
    OT_ASSERT(contacts_);
    OT_ASSERT(wallet_);
    OT_ASSERT(storage_);

    activity_.reset(new api::Activity(*contacts_, *storage_, *wallet_));
}

void OT::Init_Api()
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

    api_.reset(new api::Api(
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

void OT::Init_Blockchain()
{
    OT_ASSERT(activity_);
    OT_ASSERT(crypto_);
    OT_ASSERT(storage_);
    OT_ASSERT(wallet_)

    blockchain_.reset(
        new api::Blockchain(*activity_, *crypto_, *storage_, *wallet_));
}

void OT::Init_Config()
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

void OT::Init_Contacts()
{
    OT_ASSERT(storage_)
    OT_ASSERT(wallet_)

    contacts_.reset(new api::ContactManager(*storage_, *wallet_));
}

void OT::Init_Contracts() { wallet_.reset(new api::Wallet(*this)); }

void OT::Init_Crypto() { crypto_.reset(new CryptoEngine(*this)); }

void OT::Init_Dht()
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

    dht_.reset(api::Dht::It(config));
}

void OT::Init_Identity() { identity_.reset(new api::Identity); }

void OT::Init_Log()
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

void OT::Init_Server()
{
    if (false == server_mode_) {

        return;
    }

    OT_ASSERT(wallet_);

    server_.reset(new api::Server(server_args_, *wallet_, shutdown_));

    OT_ASSERT(server_);

    server_->Init();
}

void OT::Init_Storage()
{
    OT_ASSERT(crypto_);

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
    String defaultPlugin{};
    String archiveDirectory{};
    String encryptedDirectory{};

    if (primary_storage_plugin_.empty()) {
        defaultPlugin = config.primary_plugin_.c_str();
    } else {
        defaultPlugin = primary_storage_plugin_.c_str();
    }

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
    Config().CheckSet_str(
        STORAGE_CONFIG_KEY,
        STORAGE_CONFIG_PRIMARY_PLUGIN_KEY,
        defaultPlugin,
        config.primary_plugin_,
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
            static_cast<void (api::Dht::*)(
                const std::string&, const std::string&)>(&api::Dht::Insert),
            dht_.get(),
            std::placeholders::_1,
            std::placeholders::_2);
    }

    OT_ASSERT(crypto_);

    storage_.reset(new api::Storage(shutdown_, config, *crypto_, hash, random));
    Config().Save();
}

void OT::Init_StorageBackup()
{
    OT_ASSERT(storage_);

    storage_->InitBackup();

    if (storage_encryption_key_) {
        storage_->InitEncryptedBackup(storage_encryption_key_);
    }

    storage_->start();
}

void OT::Init_Periodic()
{
    OT_ASSERT(storage_);

    auto storage = storage_.get();
    auto now = std::time(nullptr);

    Schedule(
        nym_publish_interval_,
        [storage]() -> void {
            NymLambda nymLambda(
                [](const serializedCredentialIndex& nym) -> void {
                    OT::App().DHT().Insert(nym);
                });
            storage->MapPublicNyms(nymLambda);
        },
        now);

    Schedule(
        nym_refresh_interval_,
        [storage]() -> void {
            NymLambda nymLambda(
                [](const serializedCredentialIndex& nym) -> void {
                    OT::App().DHT().GetPublicNym(nym.nymid());
                });
            storage->MapPublicNyms(nymLambda);
        },
        (now - nym_refresh_interval_ / 2));

    Schedule(
        server_publish_interval_,
        [storage]() -> void {
            ServerLambda serverLambda(
                [](const proto::ServerContract& server) -> void {
                    OT::App().DHT().Insert(server);
                });
            storage->MapServers(serverLambda);
        },
        now);

    Schedule(
        server_refresh_interval_,
        [storage]() -> void {
            ServerLambda serverLambda(
                [](const proto::ServerContract& server) -> void {
                    OT::App().DHT().GetServerContract(server.id());
                });
            storage->MapServers(serverLambda);
        },
        (now - server_refresh_interval_ / 2));

    Schedule(
        unit_publish_interval_,
        [storage]() -> void {
            UnitLambda unitLambda(
                [](const proto::UnitDefinition& unit) -> void {
                    OT::App().DHT().Insert(unit);
                });
            storage->MapUnitDefinitions(unitLambda);
        },
        now);

    Schedule(
        unit_refresh_interval_,
        [storage]() -> void {
            UnitLambda unitLambda(
                [](const proto::UnitDefinition& unit) -> void {
                    OT::App().DHT().GetUnitDefinition(unit.id());
                });
            storage->MapUnitDefinitions(unitLambda);
        },
        (now - unit_refresh_interval_ / 2));

    periodic_.reset(new std::thread(&OT::Periodic, this));
}

void OT::Init_ZMQ()
{
    auto& config = config_[""];

    OT_ASSERT(config);

    zeromq_.reset(new api::ZMQ(*config));
}

void OT::Join()
{
    while (nullptr != instance_pointer_) {
        Log::Sleep(std::chrono::milliseconds(250));
    }
}

void OT::Periodic()
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

        if (!shutdown_.load()) {
            Log::Sleep(std::chrono::milliseconds(100));
        }
    }
}

void OT::recover()
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

void OT::Schedule(
    const time64_t& interval,
    const PeriodicTask& task,
    const time64_t& last) const
{
    // Make sure nobody is iterating while we add to the list
    std::lock_guard<std::mutex> listLock(task_list_lock_);

    periodic_task_list.push_back(TaskItem{last, interval, task});
}

const api::Server& OT::Server() const
{
    OT_ASSERT(server_);

    return *server_;
}

void OT::ServerFactory(
    const std::map<std::string, std::string>& serverArgs,
    const std::chrono::seconds gcInterval,
    const std::string& storagePlugin,
    const std::string& backupDirectory)
{
    assert(nullptr == instance_pointer_);

    instance_pointer_ = new OT(
        false,
        "",
        "",
        true,
        gcInterval,
        storagePlugin,
        backupDirectory,
        "",
        serverArgs);

    assert(nullptr != instance_pointer_);

    instance_pointer_->Init();
}

bool OT::ServerMode() const { return server_mode_; }

void OT::set_storage_encryption()
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

void OT::shutdown()
{
    shutdown_.store(true);

    if (periodic_) {
        periodic_->join();
    }

    if (server_) {
        server_->Cleanup();
    }

    if (api_) {
        api_->Cleanup();
    }

    server_.reset();
    api_.reset();
    blockchain_.reset();
    activity_.reset();
    identity_.reset();
    contacts_.reset();
    wallet_.reset();
    zeromq_.reset();
    dht_.reset();
    storage_.reset();
    crypto_.reset();
    Log::Cleanup();

    for (auto& config : config_) {
        config.second.reset();
    }

    config_.clear();
}

const std::atomic<bool>& OT::Shutdown() const { return shutdown_; }

void OT::start()
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

        server_->Start();
    }
}

api::Wallet& OT::Wallet() const
{
    OT_ASSERT(wallet_)

    return *wallet_;
}

api::ZMQ& OT::ZMQ() const
{
    OT_ASSERT(zeromq_)

    return *zeromq_;
}
}  // namespace opentxs
