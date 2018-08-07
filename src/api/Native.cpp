// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/server/Manager.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#endif
#include "opentxs/api/Legacy.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTCallback.hpp"
#include "opentxs/core/crypto/OTCaller.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "opentxs/ui/PayableList.hpp"
#include "opentxs/util/Signals.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"

#include "api/client/InternalClient.hpp"
#include "api/storage/StorageInternal.hpp"
#include "internal/api/Internal.hpp"
#include "network/DhtConfig.hpp"
#include "network/OpenDHT.hpp"
#include "storage/StorageConfig.hpp"
#include "Scheduler.hpp"

#include <atomic>
#include <ctime>
#include <limits>
#include <map>
#include <mutex>
#include <string>

#include "Native.hpp"

#define CLIENT_CONFIG_KEY "client"
#define SERVER_CONFIG_KEY "server"
#define STORAGE_CONFIG_KEY "storage"

#ifndef _PASSWORD_LEN
#define _PASSWORD_LEN 128
#endif

#define OT_METHOD "opentxs::api::implementation::Native::"

namespace
{
// If the password callback isn't set, then it uses the default ("test")
// password.
extern "C" std::int32_t default_pass_cb(
    char* buf,
    std::int32_t size,
    [[maybe_unused]] std::int32_t rwflag,
    void* userdata)
{
    std::int32_t len{0};
    const auto theSize{std::uint32_t(size)};
    const opentxs::OTPasswordData* pPWData{nullptr};
    std::string str_userdata;

    if (nullptr != userdata) {
        pPWData = static_cast<const opentxs::OTPasswordData*>(userdata);

        if (nullptr != pPWData) { str_userdata = pPWData->GetDisplayString(); }
    } else {
        str_userdata = "";
    }

    opentxs::otWarn << __FUNCTION__
                    << ": Using DEFAULT TEST PASSWORD: 'test' (for \""
                    << str_userdata << "\")\n";

    const char* tmp_passwd = "test";
    len = static_cast<std::int32_t>(strlen(tmp_passwd));

    if (len <= 0) {
        opentxs::otOut << __FUNCTION__ << ": Problem? Returning 0...\n";

        return 0;
    }

    if (len > size) { len = size; }

    const auto theLength = len;

    opentxs::OTPassword::safe_memcpy(
        buf,         // destination
        theSize,     // size of destination buffer.
        tmp_passwd,  // source
        theLength);  // length of source.

    return len;
}

// This is the function that OpenSSL calls when it wants to ask the user for his
// password. If we return 0, that's bad, that means the password caller and
// callback failed somehow.
extern "C" std::int32_t souped_up_pass_cb(
    char* buf,
    std::int32_t size,
    std::int32_t rwflag,
    void* userdata)
{
    OT_ASSERT(nullptr != userdata);

    const auto& native =
        dynamic_cast<const opentxs::api::internal::Native&>(opentxs::OT::App());
    const auto* pPWData = static_cast<const opentxs::OTPasswordData*>(userdata);
    const std::string str_userdata = pPWData->GetDisplayString();
    opentxs::OTPassword thePassword;
    bool bGotPassword = false;

    // Sometimes it's passed in, otherwise we use the global one.
    const auto providedKey = pPWData->GetCachedKey();
    auto& cachedKey =
        (nullptr == providedKey) ? native.Crypto().DefaultKey() : *providedKey;
    const bool b1 = pPWData->isForNormalNym();
    const bool b3 = !(cachedKey.isPaused());

    // For example, perhaps we need to collect a password for a symmetric key.
    // In that case, it has nothing to do with any master key, or any
    // public/private keys. It ONLY wants to do a simple password collect.
    const bool bOldSystem = pPWData->isUsingOldSystem();

    // It's for one of the normal Nyms. (NOT the master key.)
    // If it was for the master key, we'd just pop up the dialog and get the
    // master passphrase. But since it's for a NORMAL Nym, we have to call
    // OTCachedKey::GetMasterPassword. IT will pop up the dialog if it needs to,
    // by recursively calling this in master mode, and then it'll use the user
    // passphrase from that dialog to derive a key, and use THAT key to unlock
    // the actual "passphrase" (a random value) which is then passed back to
    // OpenSSL to use for the Nyms.
    if (b1 &&  // Normal Nyms, unlike Master passwords, have to look up the
               // master password first.
        !bOldSystem && b3)  // ...Unless they are still using the old system, in
                            // which case they do NOT look up the master
                            // password...
    {
        // Therefore we need to provide the password from an
        // crypto::key::LegacySymmetric stored here. (the "actual key" in the
        // crypto::key::LegacySymmetric IS the password that we are passing
        // back!)

        // So either the "actual key" is cached on a timer, from some previous
        // request like this, OR we have to pop up the passphrase dialog, ask
        // for the passphrase for the crypto::key::LegacySymmetric, and then use
        // it to GET the actual key from that crypto::key::LegacySymmetric. The
        // crypto::key::Symmetric should be stored in the OTWallet or Server,
        // which sets a pointer to itself inside the OTPasswordData class
        // statically, on initialization. That way, OTPasswordData can use that
        // pointer to get a pointer to the relevant crypto::key::LegacySymmetric
        // being used as the MASTER key.
        opentxs::otLog3 << __FUNCTION__
                        << ": Using GetMasterPassword() call. \n";
        bGotPassword = cachedKey.GetMasterPassword(
            cachedKey,
            thePassword,
            str_userdata.c_str());  // bool bVerifyTwice=false

        // NOTE: shouldn't the above call to GetMasterPassword be passing the
        // rwflag as the final parameter? Just as we see below with the call to
        // GetPasswordFromConsole. Right? Of course, it DOES generate
        // internally, if necessary, and thus it forces an "ask twice" in that
        // situation anyway. (It's that smart.) Actually that's it. The master
        // already asks twice when it's generating.
    } else {
        opentxs::otLog3 << __FUNCTION__ << ": Using OT Password Callback. \n";
        auto& caller = native.GetPasswordCaller();
        // The dialog should display this string (so the user knows what he is
        // authorizing.)
        caller.SetDisplay(
            str_userdata.c_str(),
            static_cast<std::int32_t>(str_userdata.size()));

        if (1 == rwflag) {
            opentxs::otLog4
                << __FUNCTION__
                << ": Using OT Password Callback (asks twice) for \""
                << str_userdata << "\"...\n";
            caller.callTwo();
        } else {
            opentxs::otLog4 << __FUNCTION__
                            << ": Using OT Password Callback (asks once) for \""
                            << str_userdata << "\"...\n";
            caller.callOne();
        }
        /*
            NOTICE: (For security...)

            We are using an OTPassword object to collect the password from the
            caller. (We're not passing strings back and forth.) The OTPassword
            object is where wecan centralize our efforts to scrub the memory
            clean as soon as we're done with the password. It's also designed to
            be light (no baggage) and to be passed around easily, with a
            set-size array for the data.

            Notice I am copying the password directly from the OTPassword
            object into the buffer provided to me by OpenSSL. When the
            OTPassword object goes out of scope, then it cleans up
            automatically.
        */
        bGotPassword = caller.GetPassword(thePassword);
    }

    if (false == bGotPassword) {
        opentxs::otOut
            << __FUNCTION__
            << ": Failure: (false == bGotPassword.) (Returning 0.)\n";

        return 0;
    }

    opentxs::otInfo << __FUNCTION__ << ": Success!\n";
    std::int32_t len = thePassword.isPassword() ? thePassword.getPasswordSize()
                                                : thePassword.getMemorySize();

    if (len < 0) {
        opentxs::otOut << __FUNCTION__
                       << ": <0 length password was "
                          "returned from the API password callback. "
                          "Returning 0.\n";

        return 0;
    } else if (len == 0) {
        const char* szDefault = "test";
        opentxs::otOut << __FUNCTION__
                       << ": 0 length password was "
                          "returned from the API password callback. "
                          "Substituting default password 'test'.\n";

        if (thePassword.isPassword()) {
            thePassword.setPassword(
                szDefault,
                static_cast<std::int32_t>(
                    opentxs::String::safe_strlen(szDefault, _PASSWORD_LEN)));
        } else {
            thePassword.setMemory(
                static_cast<const void*>(szDefault),
                static_cast<std::uint32_t>(
                    opentxs::String::safe_strlen(szDefault, _PASSWORD_LEN)) +
                    1);  // setMemory doesn't assume the null
                         // terminator like setPassword does.
        }

        len = thePassword.isPassword() ? thePassword.getPasswordSize()
                                       : thePassword.getMemorySize();
    }

    auto pMasterPW = pPWData->GetMasterPW();

    if (pPWData->isForCachedKey() && (nullptr != pMasterPW)) {
        *pMasterPW = thePassword;
    } else if (nullptr != buf) {
        if (len > size) { len = size; }

        const auto theSize = static_cast<std::uint32_t>(size);
        const auto theLength = static_cast<std::uint32_t>(len);

        if (thePassword.isPassword()) {
            opentxs::OTPassword::safe_memcpy(
                buf,                              // destination
                theSize,                          // size of destination buffer.
                thePassword.getPassword_uint8(),  // source
                theLength);                       // length of source.
            buf[theLength] = '\0';                // null terminator.
        } else {
            opentxs::OTPassword::safe_memcpy(
                buf,                            // destination
                theSize,                        // size of destination buffer.
                thePassword.getMemory_uint8(),  // source
                theLength);                     // length of source.
        }
    } else {
        //      OT_FAIL_MSG("This should never happen. (souped_up_pass_cb");
    }

    return len;
}
}  // namespace

namespace opentxs
{
api::internal::Native* Factory::Native(
    Flag& running,
    const ArgList& args,
    const bool recover,
    const bool serverMode,
    const std::chrono::seconds gcInterval,
    OTCaller* externalPasswordCallback)
{
    return new api::implementation::Native(
        running,
        args,
        recover,
        serverMode,
        gcInterval,
        externalPasswordCallback);
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
Native::Native(
    Flag& running,
    const ArgList& args,
    const bool recover,
    const bool serverMode,
    const std::chrono::seconds gcInterval,
    OTCaller* externalPasswordCallback)
    : running_(running)
    , recover_(recover)
    , server_mode_(serverMode)
    , gc_interval_(gcInterval)
    , config_lock_()
    , task_list_lock_()
    , signal_handler_lock_()
    , client_(nullptr)
    , config_()
    , crypto_(nullptr)
#if OT_CRYPTO_WITH_BIP39
    , seeds_(nullptr)
#endif
    , legacy_(nullptr)
    , storage_(nullptr)
#if OT_CRYPTO_WITH_BIP39
    , storage_encryption_key_(opentxs::crypto::key::Symmetric::Factory())
#endif
    , server_(nullptr)
    , zmq_context_(opentxs::network::zeromq::Context::Factory())
    , signal_handler_(nullptr)
    , server_args_(args)
    , shutdown_callback_{nullptr}
    , null_callback_{nullptr}
    , default_external_password_callback_{nullptr}
    , external_password_callback_{externalPasswordCallback}
{
    // NOTE: OT_ASSERT is not available until Init() has been called

    if (nullptr == external_password_callback_) {
        setup_default_external_password_callback();
    }

    assert(nullptr != external_password_callback_);

    for (const auto& [key, arg] : args) {
        if (key == OPENTXS_ARG_WORDS) {
            assert(2 > arg.size());
            assert(0 < arg.size());
            const auto& word = *arg.cbegin();
            word_list_.setPassword(word.c_str(), word.size());
        } else if (key == OPENTXS_ARG_PASSPHRASE) {
            assert(2 > arg.size());
            assert(0 < arg.size());
            const auto& passphrase = *arg.cbegin();
            passphrase_.setPassword(passphrase.c_str(), passphrase.size());
        } else if (key == OPENTXS_ARG_STORAGE_PLUGIN) {
            assert(2 > arg.size());
            assert(0 < arg.size());
            const auto& storagePlugin = *arg.cbegin();
            primary_storage_plugin_ = storagePlugin;
        } else if (key == OPENTXS_ARG_BACKUP_DIRECTORY) {
            assert(2 > arg.size());
            assert(0 < arg.size());
            const auto& backupDirectory = *arg.cbegin();
            archive_directory_ = backupDirectory;
        } else if (key == OPENTXS_ARG_ENCRYPTED_DIRECTORY) {
            assert(2 > arg.size());
            assert(0 < arg.size());
            const auto& encryptedDirectory = *arg.cbegin();
            encrypted_directory_ = encryptedDirectory;
        }
    }
}

const api::client::Manager& Native::Client() const
{
    if (server_mode_) { OT_FAIL; }

    OT_ASSERT(client_);

    return *client_;
}

const api::Settings& Native::Config(const std::string& path) const
{
    std::unique_lock<std::mutex> lock(config_lock_);
    auto& config = config_[path];

    if (!config) { config.reset(opentxs::Factory::Settings(String(path))); }

    OT_ASSERT(config);

    lock.unlock();

    return *config;
}

const api::Crypto& Native::Crypto() const
{
    OT_ASSERT(crypto_)

    return *crypto_;
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

INTERNAL_PASSWORD_CALLBACK* Native::GetInternalPasswordCallback() const
{
#if defined OT_TEST_PASSWORD
    otInfo << OT_METHOD << __FUNCTION__
           << ": WARNING, OT_TEST_PASSWORD *is* defined. The "
              "internal 'C'-based password callback was just "
              "requested by OT (to pass to OpenSSL). So, returning "
              "the default_pass_cb password callback, which will "
              "automatically return "
              "the 'test' password to OpenSSL, if/when it calls that "
              "callback function.\n";
    return &default_pass_cb;
#else
    return &souped_up_pass_cb;
#endif
}

OTCaller& Native::GetPasswordCaller() const
{
    OT_ASSERT(nullptr != external_password_callback_)

    return *external_password_callback_;
}

void Native::HandleSignals(ShutdownCallback* callback) const
{
    Lock lock(signal_handler_lock_);

    if (nullptr != callback) { shutdown_callback_ = callback; }

    if (false == bool(signal_handler_)) {
        signal_handler_.reset(new Signals(running_));
    }
}

void Native::Init()
{
    Init_Legacy();
    Init_Config();  // requires Init_Legacy()
    Init_Log();     // requires Init_Config()
    Init_Crypto();
    Init_Storage();  // requires Init_Legacy(), Init_Config(), Init_Crypto()
#if OT_CRYPTO_WITH_BIP39
    Init_Seeds();  // Requires Init_Crypto(), Init_Storage()
#endif
    Init_Api();  // requires Init_Legacy(), Init_Config(), Init_Crypto(),
                 // Init_Dht(), Init_Storage(),
                 // Init_Seeds()

    if (recover_) { recover(); }

    Init_Server();  // requires Init_Legacy(), Init_Config(), Init_Storage(),
                    // Init_Crypto(), Init_Log(), Init_Seeds(), Init_Dht()

    start();
}

void Native::Init_Api()
{
    auto& config = config_[""];

    OT_ASSERT(config);
    OT_ASSERT(crypto_);
#if OT_CRYPTO_WITH_BIP39
    OT_ASSERT(seeds_);
#endif
    OT_ASSERT(legacy_);

    if (server_mode_) { return; }

    client_.reset(opentxs::Factory::ClientManager(
        running_,
        *config,
        *crypto_,
#if OT_CRYPTO_WITH_BIP39
        *seeds_,
#endif
        *legacy_,
        *storage_,
        zmq_context_,
        legacy_->ClientDataFolder(),
        0));  // TODO

    OT_ASSERT(client_);
}

void Native::Init_Config()
{
    OT_ASSERT(legacy_)

    String strConfigFilePath = legacy_->ConfigFilePath().c_str();
    config_[""].reset(opentxs::Factory::Settings(strConfigFilePath));
}

void Native::Init_Crypto() { crypto_.reset(opentxs::Factory::Crypto()); }

void Native::Init_Legacy()
{
    if (server_mode_) {
        legacy_.reset(opentxs::Factory::Legacy(SERVER_CONFIG_KEY));
    } else {
        legacy_.reset(opentxs::Factory::Legacy(CLIENT_CONFIG_KEY));
    }

    OT_ASSERT(legacy_);
}

void Native::Init_Log()
{
    std::string type{};

    if (server_mode_) {
        type = SERVER_CONFIG_KEY;
    } else {
        type = CLIENT_CONFIG_KEY;
    }

    if (false == Log::Init(Config(), type.c_str())) { abort(); }
}

#if OT_CRYPTO_WITH_BIP39
void Native::Init_Seeds()
{
    OT_ASSERT(crypto_);
    OT_ASSERT(storage_);

    seeds_.reset(opentxs::Factory::HDSeed(
        crypto_->Symmetric(),
        *storage_,
        crypto_->BIP32(),
        crypto_->BIP39(),
        crypto_->AES()));

    OT_ASSERT(seeds_);
}
#endif

void Native::Init_Server()
{
    if (false == server_mode_) { return; }

    OT_ASSERT(crypto_);
    OT_ASSERT(seeds_);
    OT_ASSERT(storage_);

    server_.reset(opentxs::Factory::ServerManager(
        running_,
        server_args_,
        *storage_,
        *crypto_,
#if OT_CRYPTO_WITH_BIP39
        *seeds_,
#endif
        *legacy_,
        Config(),
        zmq_context_,
        legacy_->ServerDataFolder(),
        1));  // TODO

    OT_ASSERT(server_);
}

void Native::Init_Storage()
{
    OT_ASSERT(crypto_);

    Digest hash = std::bind(
        static_cast<bool (api::crypto::Hash::*)(
            const std::uint32_t, const std::string&, std::string&) const>(
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
                 path,
                 legacy_->DataFolderPath(),
                 OTFolders::Common().Get(),
                 ".temp",
                 "",
                 "")) {
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

    /* Storage is always initialized before Dht
        if (dht_) {
            config.dht_callback_ = std::bind(
                static_cast<void (api::network::Dht::*)(
                    const std::string&, const std::string&) const>(
                    &api::network::Dht::Insert),
                dht_.get(),
                std::placeholders::_1,
                std::placeholders::_2);
        }
    */

    OT_ASSERT(crypto_);

    storage_.reset(opentxs::Factory::Storage(
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

    storage_->InitBackup();

#if OT_CRYPTO_WITH_BIP39
    if (storage_encryption_key_.get()) {
        storage_->InitEncryptedBackup(storage_encryption_key_);
    }
#endif

    storage_->start();
}

const api::Legacy& Native::Legacy() const
{
    OT_ASSERT(legacy_)

    return *legacy_;
}

void Native::recover()
{
    OT_ASSERT(client_);
    OT_ASSERT(crypto_);
    OT_ASSERT(recover_);
    OT_ASSERT(storage_);
    OT_ASSERT(0 < word_list_.getPasswordSize());

    auto& api = client_->OTAPI();
    const auto fingerprint = api.Wallet_ImportSeed(word_list_, passphrase_);

    if (fingerprint.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to import seed."
              << std::endl;
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Imported seed " << fingerprint
              << std::endl;
    }
}

const api::server::Manager& Native::Server() const
{
    OT_ASSERT(server_);

    return *server_;
}

bool Native::ServerMode() const { return server_mode_; }

void Native::set_storage_encryption()
{
#if OT_CRYPTO_WITH_BIP39
    OT_ASSERT(seeds_);

    auto seed = seeds_->DefaultSeed();

    if (seed.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": No default seed." << std::endl;
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Default seed is: " << seed
              << std::endl;
    }

    std::shared_ptr<proto::AsymmetricKey> rawKey{nullptr};

    if (server_mode_) {
        OT_ASSERT(server_)

        rawKey = server_->Seeds().GetStorageKey(seed);
    } else {
        OT_ASSERT(client_)

        rawKey = client_->Seeds().GetStorageKey(seed);
    }

    if (false == bool(rawKey)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load encryption key."
              << std::endl;
    }

    storage_encryption_key_ = crypto_->GetStorageKey(*rawKey);

    if (storage_encryption_key_.get()) {
        otWarn << OT_METHOD << __FUNCTION__ << ": Obtained storage key "
               << storage_encryption_key_->ID()->str() << std::endl;
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load storage key "
              << seed << std::endl;
    }
#endif
}

void Native::setup_default_external_password_callback()
{
    // NOTE: OT_ASSERT is not available yet because we're too early
    // in the startup process

    null_callback_.reset(opentxs::Factory::NullCallback());

    assert(null_callback_);

    default_external_password_callback_.reset(new OTCaller);

    assert(default_external_password_callback_);

    default_external_password_callback_->setCallback(null_callback_.get());

    assert(default_external_password_callback_->isCallbackSet());

    external_password_callback_ = default_external_password_callback_.get();
}

void Native::shutdown()
{
    running_.Off();

    if (nullptr != shutdown_callback_) {
        ShutdownCallback& callback = *shutdown_callback_;
        callback();
    }

    if (false == bool(server_)) {
        OT_ASSERT(client_);

        auto wallet = client_->OTAPI().GetWallet(nullptr);

        OT_ASSERT(nullptr != wallet);

        wallet->SaveWallet();
    }

    server_.reset();
    client_.reset();
    storage_.reset();
    crypto_.reset();
    Log::Cleanup();

    for (auto& config : config_) { config.second.reset(); }

    config_.clear();
}

void Native::start()
{
    if (false == server_mode_) {
        OT_ASSERT(client_);

        auto wallet = client_->StartWallet();

        OT_ASSERT(nullptr != wallet);

        if (false == encrypted_directory_.empty()) { set_storage_encryption(); }

        wallet->SaveWallet();
    }

    Init_StorageBackup();

    OT_ASSERT(storage_);

    storage_->UpgradeNyms();

    if (false == server_mode_) {
        OT_ASSERT(client_);

        client_->StartContacts();
        client_->StartActivity();
    }

    if (server_mode_) {
        OT_ASSERT(server_);

        server_->Start();
    }
}
}  // namespace opentxs::api::implementation
