// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::implementation
{
class Context final : api::internal::Context, Lockable, Periodic
{
public:
    const api::client::internal::Manager& Client(
        const int instance) const final;
    std::size_t Clients() const final { return client_.size(); }
    const api::Settings& Config(const std::string& path) const final;
    const api::Crypto& Crypto() const final;
    void HandleSignals(ShutdownCallback* shutdown) const final;
    const api::Legacy& Legacy() const noexcept final { return *legacy_; }
    proto::RPCResponse RPC(const proto::RPCCommand& command) const final;
    const api::server::Manager& Server(const int instance) const final;
    std::size_t Servers() const final { return server_.size(); }
    const api::client::internal::Manager& StartClient(
        const ArgList& args,
        const int instance) const final;
#if OT_CRYPTO_WITH_BIP39
    const api::client::internal::Manager& StartClient(
        const ArgList& args,
        const int instance,
        const std::string& recoverWords,
        const std::string& recoverPassphrase) const final;
#endif  // OT_CRYPTO_WITH_BIP39
    const api::server::Manager& StartServer(
        const ArgList& args,
        const int instance,
        const bool inproc) const final;
    const api::network::ZAP& ZAP() const final;
    const opentxs::network::zeromq::Context& ZMQ() const final
    {
        return zmq_context_.get();
    }

    OTCaller& GetPasswordCaller() const final;

private:
    friend opentxs::Factory;

    typedef std::map<std::string, std::unique_ptr<api::Settings>> ConfigMap;

    const std::chrono::seconds gc_interval_{0};
    OTPassword word_list_{};
    OTPassword passphrase_{};
    mutable std::mutex config_lock_;
    mutable std::mutex task_list_lock_;
    mutable std::mutex signal_handler_lock_;
    mutable ConfigMap config_;
    OTZMQContext zmq_context_;
    mutable std::unique_ptr<Signals> signal_handler_;
    std::unique_ptr<api::internal::Log> log_;
    std::unique_ptr<api::Crypto> crypto_;
    std::unique_ptr<api::Legacy> legacy_;
    std::unique_ptr<api::network::ZAP> zap_;
    const ArgList args_;
    mutable ShutdownCallback* shutdown_callback_{nullptr};
    std::unique_ptr<OTCallback> null_callback_{nullptr};
    std::unique_ptr<OTCaller> default_external_password_callback_{nullptr};
    OTCaller* external_password_callback_{nullptr};
    mutable std::unique_ptr<opentxs::PIDFile> pid_;
    mutable std::vector<std::unique_ptr<api::server::Manager>> server_;
    mutable std::vector<std::unique_ptr<api::client::internal::Manager>>
        client_;
    std::unique_ptr<rpc::internal::RPC> rpc_;

    static int client_instance(const int count);
    static int server_instance(const int count);
    static std::string get_arg(const ArgList& args, const std::string& argName);

    void init_pid(const Lock& lock) const;
    const ArgList merge_arglist(const ArgList& args) const;
    void start_client(const Lock& lock, const ArgList& args) const;
    void start_server(const Lock& lock, const ArgList& args) const;

    void Init_Crypto();
    void Init_Log(const std::int32_t argLevel);
    void Init_Zap();
    void Init() final;
    void setup_default_external_password_callback();
    void shutdown() final;

    explicit Context(
        Flag& running,
        const ArgList& args,
        const std::chrono::seconds gcInterval,
        OTCaller* externalPasswordCallback = nullptr);
    Context() = delete;
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;

    ~Context() final;
};
}  // namespace opentxs::api::implementation
