// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_API_IMPLEMENTATION_NATIVE_HPP
#define OPENTXS_CORE_API_IMPLEMENTATION_NATIVE_HPP

#include "Internal.hpp"

namespace
{
extern "C" {
INTERNAL_PASSWORD_CALLBACK default_pass_cb;
INTERNAL_PASSWORD_CALLBACK souped_up_pass_cb;
}
}  // namespace

namespace opentxs::api::implementation
{
/** \brief Singlton class for providing an interface to process-level resources.
 *  \ingroup native
 */
class Native : virtual public api::internal::Native
{
public:
    const api::client::Client& Client() const override;
    const api::Settings& Config(
        const std::string& path = std::string("")) const override;
    const api::Crypto& Crypto() const override;
    const api::storage::Storage& DB() const override;
    const api::network::Dht& DHT() const override;
    void HandleSignals(ShutdownCallback* shutdown) const override;
    const api::Identity& Identity() const override;
    const api::Legacy& Legacy() const override;
    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution. */
    void Schedule(
        const std::chrono::seconds& interval,
        const PeriodicTask& task,
        const std::chrono::seconds& last =
            std::chrono::seconds(0)) const override;
    const api::Server& Server() const override;
    bool ServerMode() const override;
    const api::Wallet& Wallet() const override;
    const api::network::ZMQ& ZMQ() const override;

    INTERNAL_PASSWORD_CALLBACK* GetInternalPasswordCallback() const override;
    OTCaller& GetPasswordCaller() const override;

private:
    friend Factory;
    friend class opentxs::OT;

    /** Last performed, Interval, Task */
    typedef std::tuple<time64_t, time64_t, PeriodicTask> TaskItem;
    typedef std::list<TaskItem> TaskList;
    typedef std::map<std::string, std::unique_ptr<api::Settings>> ConfigMap;

    Flag& running_;
    const bool recover_{false};
    const bool server_mode_{false};
    std::int64_t nym_publish_interval_{0};
    std::int64_t nym_refresh_interval_{0};
    std::int64_t server_publish_interval_{0};
    std::int64_t server_refresh_interval_{0};
    std::int64_t unit_publish_interval_{0};
    std::int64_t unit_refresh_interval_{0};
    const std::chrono::seconds gc_interval_{0};
    OTPassword word_list_{};
    OTPassword passphrase_{};
    std::string primary_storage_plugin_{};
    std::string archive_directory_{};
    std::string encrypted_directory_{};
    mutable std::mutex config_lock_;
    mutable std::mutex task_list_lock_;
    mutable std::mutex signal_handler_lock_;
    mutable TaskList periodic_task_list;
    std::unique_ptr<api::client::internal::Client> client_;
    mutable ConfigMap config_;
    std::unique_ptr<api::Crypto> crypto_;
    std::unique_ptr<api::network::Dht> dht_;
    std::unique_ptr<api::Identity> identity_;
    std::unique_ptr<api::Legacy> legacy_;
    std::unique_ptr<api::storage::StorageInternal> storage_;
    std::unique_ptr<api::Wallet> wallet_;
    std::unique_ptr<api::network::ZMQ> zeromq_;
    std::unique_ptr<std::thread> periodic_;
#if OT_CRYPTO_WITH_BIP39
    OTSymmetricKey storage_encryption_key_;
#endif
    std::unique_ptr<api::Server> server_;
    OTZMQContext zmq_context_;
    mutable std::unique_ptr<Signals> signal_handler_;
    const ArgList server_args_;
    mutable ShutdownCallback* shutdown_callback_{nullptr};
    std::unique_ptr<OTCallback> null_callback_{nullptr};
    std::unique_ptr<OTCaller> default_external_password_callback_{nullptr};
    OTCaller* external_password_callback_{nullptr};

    explicit Native(
        Flag& running,
        const ArgList& args,
        const bool recover,
        const bool serverMode,
        const std::chrono::seconds gcInterval,
        OTCaller* externalPasswordCallback = nullptr);
    Native() = delete;
    Native(const Native&) = delete;
    Native(Native&&) = delete;
    Native& operator=(const Native&) = delete;
    Native& operator=(Native&&) = delete;

    String get_primary_storage_plugin(
        const StorageConfig& config,
        bool& migrate,
        String& previous) const;

    void setup_default_external_password_callback();

    void Init_Api();
    void Init_Config();
    void Init_Contracts();
    void Init_Crypto();
    void Init_Dht();
    void Init_Identity();
    void Init_Legacy();
    void Init_Log();
    void Init_Periodic();
    void Init_Server();
    void Init_Storage();
    void Init_StorageBackup();
    void Init_ZMQ();
    void Init() override;
    void Periodic();
    void recover();
    void set_storage_encryption();
    void shutdown() override;
    void start();

    ~Native();
};
}  // namespace opentxs::api::implementation
#endif  // OPENTXS_CORE_API_IMPLEMENTATION_NATIVE_HPP
