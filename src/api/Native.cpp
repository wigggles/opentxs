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
#include "opentxs/api/network/ZAP.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#endif
#include "opentxs/api/Legacy.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/core/crypto/OTCallback.hpp"
#include "opentxs/core/crypto/OTCaller.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
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
#include "opentxs/util/PIDFile.hpp"
#include "opentxs/util/Signals.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"

#include "internal/api/client/Client.hpp"
#include "internal/api/storage/Storage.hpp"
#include "internal/api/Api.hpp"
#include "internal/rpc/RPC.hpp"
#include "network/OpenDHT.hpp"
#include "storage/StorageConfig.hpp"
#include "Periodic.hpp"
#include "Scheduler.hpp"

#include <algorithm>
#include <atomic>
#include <ctime>
#include <limits>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "Native.hpp"

// #define OT_METHOD "opentxs::api::implementation::Native::"

namespace opentxs
{
api::internal::Native* Factory::Native(
    Flag& running,
    const ArgList& args,
    const std::chrono::seconds gcInterval,
    OTCaller* externalPasswordCallback)
{
    return new api::implementation::Native(
        running, args, gcInterval, externalPasswordCallback);
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
Native::Native(
    Flag& running,
    const ArgList& args,
    const std::chrono::seconds gcInterval,
    OTCaller* externalPasswordCallback)
    : api::internal::Native()
    , Lockable()
    , Periodic(running)
    , gc_interval_(gcInterval)
    , config_lock_()
    , task_list_lock_()
    , signal_handler_lock_()
    , config_()
    , zmq_context_(opentxs::network::zeromq::Context::Factory())
    , signal_handler_(nullptr)
    , log_(opentxs::Factory::Log(
          zmq_context_,
          get_arg(args, OPENTXS_ARG_LOGENDPOINT)))
    , crypto_(nullptr)
    , legacy_(opentxs::Factory::Legacy())
    , zap_(nullptr)
    , args_(args)
    , shutdown_callback_{nullptr}
    , null_callback_{nullptr}
    , default_external_password_callback_{nullptr}
    , external_password_callback_{externalPasswordCallback}
    , pid_(nullptr)
    , server_()
    , client_()
    , rpc_(opentxs::Factory::RPC(*this))
{
    // NOTE: OT_ASSERT is not available until Init() has been called
    assert(legacy_);
    assert(log_);

    if (nullptr == external_password_callback_) {
        setup_default_external_password_callback();
    }

    assert(nullptr != external_password_callback_);

    const auto& word = get_arg(args, OPENTXS_ARG_WORDS);
    if (!word.empty()) { word_list_.setPassword(word.c_str(), word.size()); }

    const auto& passphrase = get_arg(args, OPENTXS_ARG_PASSPHRASE);
    if (!passphrase.empty()) {
        passphrase_.setPassword(passphrase.c_str(), passphrase.size());
    }

    assert(rpc_);
}

int Native::client_instance(const int count)
{
    // NOTE: Instance numbers must not collide between clients and servers.
    // Clients use even numbers and servers use odd numbers.
    return (2 * count);
}

const api::client::Manager& Native::Client(const int instance) const
{
    auto& output = client_.at(instance);

    OT_ASSERT(output);

    return *output;
}

const api::Settings& Native::Config(const std::string& path) const
{
    std::unique_lock<std::mutex> lock(config_lock_);
    auto& config = config_[path];

    if (!config) {
        config.reset(opentxs::Factory::Settings(String::Factory(path)));
    }

    OT_ASSERT(config);

    lock.unlock();

    return *config;
}

const api::Crypto& Native::Crypto() const
{
    OT_ASSERT(crypto_)

    return *crypto_;
}

std::string Native::get_arg(const ArgList& args, const std::string& argName)
{
    auto argIt = args.find(argName);

    if (args.end() != argIt) {
        const auto& argItems = argIt->second;

        OT_ASSERT(2 > argItems.size());
        OT_ASSERT(0 < argItems.size());

        return *argItems.cbegin();
    }

    return {};
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
    std::int32_t argLevel{-2};

    try {
        argLevel = std::stoi(get_arg(args_, OPENTXS_ARG_LOGLEVEL));
    } catch (...) {
    }

    Init_Log(argLevel);
    Init_Crypto();
    Init_Zap();
}

void Native::Init_Crypto()
{
    crypto_.reset(
        opentxs::Factory::Crypto(Config(legacy_->CryptoConfigFilePath())));
}

void Native::Init_Log(const std::int32_t argLevel)
{
    OT_ASSERT(legacy_)

    const auto& config = Config(legacy_->LogConfigFilePath());
    const auto init = Log::Init(config);
    bool notUsed{false};
    std::int64_t level{0};

    if (-1 > argLevel) {
        config.CheckSet_long(
            String::Factory("logging"),
            String::Factory(OPENTXS_ARG_LOGLEVEL),
            0,
            level,
            notUsed);
    } else {
        config.Set_long(
            String::Factory("logging"),
            String::Factory(OPENTXS_ARG_LOGLEVEL),
            argLevel,
            notUsed);
        level = argLevel;
    }

    Log::SetLogLevel(static_cast<std::int32_t>(level));

    if (false == init) { abort(); }
}

void Native::init_pid(const Lock& lock) const
{
    if (false == bool(pid_)) {
        OT_ASSERT(legacy_);

        pid_.reset(opentxs::Factory::PIDFile(legacy_->PIDFilePath()));

        OT_ASSERT(pid_);

        pid_->Open();
    }

    OT_ASSERT(pid_);
    OT_ASSERT(pid_->isOpen());
}

void Native::Init_Zap()
{
    zap_.reset(opentxs::Factory::ZAP(zmq_context_));

    OT_ASSERT(zap_);
}

const ArgList Native::merge_arglist(const ArgList& args) const
{
    ArgList arguments{args_};

    for (const auto& [arg, val] : args) { arguments[arg] = val; }

    return arguments;
}

proto::RPCResponse Native::RPC(const proto::RPCCommand& command) const
{
    OT_ASSERT(rpc_);

    return rpc_->Process(command);
}

int Native::server_instance(const int count)
{
    // NOTE: Instance numbers must not collide between clients and servers.
    // Clients use even numbers and servers use odd numbers.
    return (2 * count) + 1;
}

const api::server::Manager& Native::Server(const int instance) const
{
    auto& output = server_.at(instance);

    OT_ASSERT(output);

    return *output;
}

void Native::setup_default_external_password_callback()
{
    // NOTE: OT_ASSERT is not available yet because we're too early
    // in the startup process

    null_callback_.reset(opentxs::Factory::NullCallback());

    assert(null_callback_);

    default_external_password_callback_.reset(new OTCaller);

    assert(default_external_password_callback_);

    default_external_password_callback_->SetCallback(null_callback_.get());

    assert(default_external_password_callback_->HaveCallback());

    external_password_callback_ = default_external_password_callback_.get();
}

void Native::shutdown()
{
    running_.Off();

    if (nullptr != shutdown_callback_) {
        ShutdownCallback& callback = *shutdown_callback_;
        callback();
    }

    server_.clear();
    client_.clear();
    crypto_.reset();
    Log::Cleanup();

    for (auto& config : config_) { config.second.reset(); }

    config_.clear();
}

void Native::start_client(const Lock& lock, const ArgList& args) const
{
    OT_ASSERT(verify_lock(lock))
    OT_ASSERT(crypto_);
    OT_ASSERT(legacy_);

    init_pid(lock);
    auto merged_args = merge_arglist(args);
    const int next = client_.size();
    const auto instance = client_instance(next);
    client_.emplace_back(opentxs::Factory::ClientManager(
        *this,
        running_,
        merged_args,
        Config(legacy_->ClientConfigFilePath(next)),
        *crypto_,
        zmq_context_,
        legacy_->ClientDataFolder(next),
        instance));
}

const api::client::Manager& Native::StartClient(
    const ArgList& args,
    const int instance) const
{
    Lock lock(lock_);

    const std::size_t count = std::max(0, instance);
    const std::size_t effective = std::min(count, client_.size());

    if (effective == client_.size()) {
        ArgList arguments{args};
        start_client(lock, arguments);
    }

    const auto& output = client_.at(effective);

    OT_ASSERT(output)

    return *output;
}

void Native::start_server(const Lock& lock, const ArgList& args) const
{
    OT_ASSERT(verify_lock(lock))
    OT_ASSERT(crypto_);

    init_pid(lock);
    const auto merged_args = merge_arglist(args);
    const auto next{server_.size()};
    const auto instance{server_instance(next)};
    server_.emplace_back(opentxs::Factory::ServerManager(
        *this,
        running_,
        merged_args,
        *crypto_,
        Config(legacy_->ServerConfigFilePath(next)),
        zmq_context_,
        legacy_->ServerDataFolder(next),
        instance));
}

const api::server::Manager& Native::StartServer(
    const ArgList& args,
    const int instance,
    const bool inproc) const
{
    Lock lock(lock_);

    const std::size_t count = std::max(0, instance);
    const std::size_t effective = std::min(count, server_.size());

    if (effective == server_.size()) {
        ArgList arguments{args};

        if (inproc) {
            auto& inprocSet = arguments[OPENTXS_ARG_INPROC];
            inprocSet.clear();
            inprocSet.insert(std::to_string(server_instance(effective)));
        }

        start_server(lock, arguments);
    }

    const auto& output = server_.at(effective);

    OT_ASSERT(output)

    return *output;
}

const api::network::ZAP& Native::ZAP() const
{
    OT_ASSERT(zap_);

    return *zap_;
}

Native::~Native()
{
    client_.clear();
    server_.clear();

    if (pid_) { pid_->Close(); }

    LogSource::Shutdown();
    log_.reset();
}
}  // namespace opentxs::api::implementation
