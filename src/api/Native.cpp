// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Api.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/api/Blockchain.hpp"
#endif
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Legacy.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Server.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/UI.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTCallback.hpp"
#include "opentxs/core/crypto/OTCaller.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/crypto/Bip39.hpp"
#endif
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
#include "api/network/Dht.hpp"
#include "api/network/ZMQ.hpp"
#include "api/storage/StorageInternal.hpp"
#include "api/ContactManager.hpp"
#include "api/NativeInternal.hpp"
#include "network/DhtConfig.hpp"
#include "network/OpenDHT.hpp"
#include "storage/StorageConfig.hpp"

#include <atomic>
#include <ctime>
#include <cstdint>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>

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
#if OT_CRYPTO_SUPPORTED_KEY_HD
    , blockchain_(nullptr)
#endif
    , config_()
    , contacts_(nullptr)
    , crypto_(nullptr)
    , dht_(nullptr)
    , identity_(nullptr)
    , legacy_(nullptr)
    , storage_(nullptr)
    , wallet_(nullptr)
    , zeromq_(nullptr)
    , periodic_(nullptr)
#if OT_CRYPTO_WITH_BIP39
    , storage_encryption_key_(opentxs::crypto::key::Symmetric::Factory())
#endif
    , server_(nullptr)
    , ui_(nullptr)
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

const api::Activity& Native::Activity() const
{
    OT_ASSERT(activity_)

    return *activity_;
}

const api::Api& Native::API() const
{
    if (server_mode_) { OT_FAIL; }

    OT_ASSERT(api_);

    return *api_;
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
const api::Blockchain& Native::Blockchain() const
{
    OT_ASSERT(blockchain_)

    return *blockchain_;
}
#endif

const api::Settings& Native::Config(const std::string& path) const
{
    std::unique_lock<std::mutex> lock(config_lock_);
    auto& config = config_[path];

    if (!config) { config.reset(Factory::Settings(String(path))); }

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

const api::Identity& Native::Identity() const
{
    OT_ASSERT(identity_)

    return *identity_;
}

void Native::Init()
{
    Init_Legacy();
    Init_Config();  // requires Init_Legacy()
    Init_Log();     // requires Init_Config()
    Init_Crypto();
    Init_Storage();    // requires Init_Legacy(), Init_Config(), Init_Crypto()
    Init_ZMQ();      // requires Init_Config()
    Init_Contracts();
    Init_Dht();       // requires Init_Config()
    Init_Identity();  // requires Init_Contracts()
    Init_Contacts();  // requires Init_Contracts(), Init_Storage(), Init_ZMQ()
    Init_Activity();  // requires Init_Storage(), Init_Contacts(),
                       // Init_Contracts(), Init_Legacy()
#if OT_CRYPTO_SUPPORTED_KEY_HD
    Init_Blockchain();  // requires Init_Storage(), Init_Crypto(),
                        // Init_Contracts(), Init_Activity()
#endif
    Init_Api();  // requires Init_Legacy(), Init_Config(), Init_Crypto(),
                 // Init_Contracts(), Init_Identity(), Init_Storage(),
                 // Init_ZMQ(), Init_Contacts() Init_Activity()
    if (!server_mode_) {
        Init_UI();  // requires Init_Activity(), Init_Contacts(), Init_Api(),
                    // Init_Storage(), Init_ZMQ(), Init_Legacy()
    }

    if (recover_) { recover(); }

    Init_Server();  // requires Init_Legacy(), Init_Config(), Init_Storage(),
                    // Init_Crypto(), Init_Contracts(), Init_Log(),
                    // Init_Contracts()

    start();
}

void Native::Init_Activity()
{
    OT_ASSERT(legacy_);
    OT_ASSERT(contacts_);
    OT_ASSERT(wallet_);
    OT_ASSERT(storage_);

    activity_.reset(Factory::Activity(
        *legacy_, *contacts_, *storage_, *wallet_, zmq_context_));
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
    OT_ASSERT(legacy_);

    if (server_mode_) { return; }

    api_.reset(Factory::Api(
        running_,
        *activity_,
        *config,
        *contacts_,
        *crypto_,
        *identity_,
        *legacy_,
        *storage_,
        *wallet_,
        *zeromq_));

    OT_ASSERT(api_);
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
void Native::Init_Blockchain()
{
    OT_ASSERT(activity_);
    OT_ASSERT(crypto_);
    OT_ASSERT(storage_);
    OT_ASSERT(wallet_)

    blockchain_.reset(
        Factory::Blockchain(*activity_, *crypto_, *storage_, *wallet_));
}
#endif

void Native::Init_Config()
{
    OT_ASSERT(legacy_)

    String strConfigFilePath = legacy_->ConfigFilePath().c_str();
    config_[""].reset(Factory::Settings(strConfigFilePath));
}

void Native::Init_Contacts()
{
    OT_ASSERT(storage_)
    OT_ASSERT(wallet_)

    contacts_.reset(new api::implementation::ContactManager(
        *storage_, *wallet_, zmq_context_.get()));
}

void Native::Init_Contracts()
{
    wallet_.reset(Factory::Wallet(*this, zmq_context_));
}

void Native::Init_Crypto() { crypto_.reset(Factory::Crypto(*this)); }

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

    identity_.reset(Factory::Identity(*wallet_));
}

void Native::Init_Legacy()
{
    if (server_mode_) {
        legacy_.reset(Factory::Legacy(SERVER_CONFIG_KEY));
    } else {
        legacy_.reset(Factory::Legacy(CLIENT_CONFIG_KEY));
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

void Native::Init_Server()
{
    if (false == server_mode_) { return; }

    OT_ASSERT(crypto_);
    OT_ASSERT(storage_);
    OT_ASSERT(wallet_);

    server_.reset(Factory::ServerAPI(
        server_args_,
        *crypto_,
        *legacy_,
        Config(),
        *storage_,
        *wallet_,
        running_,
        zmq_context_));

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

    storage_.reset(Factory::Storage(
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

void Native::Init_UI()
{
    OT_ASSERT(activity_)
    OT_ASSERT(api_)
    OT_ASSERT(contacts_)
    OT_ASSERT(legacy_)
    OT_ASSERT(storage_)
    OT_ASSERT(wallet_)
    OT_ASSERT(zeromq_)

    ui_.reset(Factory::UI(
        api_->Sync(),
        *wallet_,
        api_->Workflow(),
        *zeromq_,
        *storage_,
        *activity_,
        *contacts_,
        *legacy_,
        zmq_context_,
        running_));

    OT_ASSERT(ui_);
}

void Native::Init_ZMQ()
{
    auto& config = config_[""];

    OT_ASSERT(config);

    zeromq_.reset(
        new api::network::implementation::ZMQ(zmq_context_, *config, running_));
}

const api::Legacy& Native::Legacy() const
{
    OT_ASSERT(legacy_)

    return *legacy_;
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
        if (storage_) { storage_->RunGC(); }

        if (running_) { Log::Sleep(std::chrono::milliseconds(100)); }
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
#if OT_CRYPTO_WITH_BIP39
    OT_ASSERT(crypto_);

    auto seed = crypto_->BIP39().DefaultSeed();

    if (seed.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": No default seed." << std::endl;
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Default seed is: " << seed
              << std::endl;
    }

    storage_encryption_key_ = crypto_->GetStorageKey(seed);

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

    null_callback_.reset(Factory::NullCallback());

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

    if (periodic_) { periodic_->join(); }

    if (false == bool(server_)) {
        OT_ASSERT(api_);

        auto wallet = api_->OTAPI().GetWallet(nullptr);

        OT_ASSERT(nullptr != wallet);

        wallet->SaveWallet();
    }

    server_.reset();
    ui_.reset();
    api_.reset();
#if OT_CRYPTO_SUPPORTED_KEY_HD
    blockchain_.reset();
#endif
    activity_.reset();
    contacts_.reset();
    identity_.reset();
    dht_.reset();
    wallet_.reset();
    zeromq_.reset();
    storage_.reset();
    crypto_.reset();
    Log::Cleanup();

    for (auto& config : config_) { config.second.reset(); }

    config_.clear();
}

void Native::start()
{
    OT_ASSERT(activity_);
    OT_ASSERT(contacts_);

    if (false == server_mode_) {
        OT_ASSERT(api_);

        const bool loaded = api_->OTAPI().LoadWallet();

        OT_ASSERT(loaded);

        auto wallet = api_->OTAPI().GetWallet(nullptr);

        OT_ASSERT(nullptr != wallet);

        if (false == encrypted_directory_.empty()) { set_storage_encryption(); }

        wallet->SaveWallet();
    }

    Init_StorageBackup();

    OT_ASSERT(storage_);

    storage_->UpgradeNyms();
    dynamic_cast<ContactManager&>(*contacts_).start();
    activity_->MigrateLegacyThreads();
    Init_Periodic();

    if (server_mode_) {
        OT_ASSERT(server_);

        server_->Start();
    }
}

const api::UI& Native::UI() const
{
    OT_ASSERT(ui_)

    return *ui_;
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

Native::~Native() {}
}  // namespace opentxs::api::implementation
