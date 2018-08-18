// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class Manager final : opentxs::api::client::internal::Manager,
                      opentxs::api::implementation::Scheduler,
                      api::implementation::StorageParent
{
public:
    const api::client::Activity& Activity() const override;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const api::client::Blockchain& Blockchain() const override;
#endif
    const api::client::Cash& Cash() const override;
    const api::Settings& Config() const override { return config_; }
    const api::client::Contacts& Contacts() const override;
    const api::Crypto& Crypto() const override { return crypto_; }
    const std::string& DataFolder() const override { return data_folder_; }
    const api::network::Dht& DHT() const override;
    const OTAPI_Exec& Exec(const std::string& wallet = "") const override;
    const api::Factory& Factory() const override;
    int Instance() const override { return instance_; }
    std::recursive_mutex& Lock(
        const Identifier& nymID,
        const Identifier& serverID) const override;
    const OT_API& OTAPI(const std::string& wallet = "") const override;
    const api::client::Pair& Pair() const override;
    void Schedule(
        const std::chrono::seconds& interval,
        const PeriodicTask& task,
        const std::chrono::seconds& last =
            std::chrono::seconds(0)) const override
    {
        Scheduler::Schedule(interval, task, last);
    }
#if OT_CRYPTO_WITH_BIP39
    const api::HDSeed& Seeds() const override;
#endif
    const client::ServerAction& ServerAction() const override;
    const api::storage::Storage& Storage() const override;
    const client::Sync& Sync() const override;
    const api::client::UI& UI() const override;
    const api::Wallet& Wallet() const override;
    const client::Workflow& Workflow() const override;
    const opentxs::network::zeromq::Context& ZeroMQ() const override
    {
        return zmq_context_;
    }
    const api::network::ZMQ& ZMQ() const override;

    void StartActivity() override;
    void StartContacts() override;
    opentxs::OTWallet* StartWallet() override;

    ~Manager();

private:
    friend opentxs::Factory;

    const opentxs::network::zeromq::Context& zmq_context_;
    const int instance_{0};
#if OT_CRYPTO_WITH_BIP39
    std::unique_ptr<api::HDSeed> seeds_;
#endif
    std::unique_ptr<api::Factory> factory_;
    std::unique_ptr<api::Wallet> wallet_;        // Depends on seeds_, factory_
    std::unique_ptr<api::network::ZMQ> zeromq_;  // Depends on wallet_
    std::unique_ptr<api::Identity> identity_;
    std::unique_ptr<api::client::internal::Contacts> contacts_;
    std::unique_ptr<api::client::internal::Activity> activity_;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    std::unique_ptr<api::client::Blockchain> blockchain_;
#endif
    std::unique_ptr<api::client::Workflow> workflow_;
    std::unique_ptr<OT_API> ot_api_;  // Depends on activity_, config_,
                                      // contacts_, factory_, seeds_, wallet_,
                                      // workflow_
    std::unique_ptr<OTAPI_Exec> otapi_exec_;
    std::unique_ptr<api::client::Cash> cash_;
    std::unique_ptr<api::client::ServerAction> server_action_;
    std::unique_ptr<api::client::Sync> sync_;  // Depends on ot_api_,
                                               // otapi_exec_, contacts_,
                                               // server_action_, wallet_,
                                               // workflow_, zeromq_
    std::unique_ptr<api::client::UI> ui_;
    std::unique_ptr<api::client::Pair> pair_;  // Depends on sync_,
                                               // server_action_, wallet_,
                                               // ot_api_, otapi_exec_, zeromq_
    std::unique_ptr<api::network::Dht> dht_;
    mutable std::recursive_mutex lock_;
    mutable std::mutex map_lock_;
    mutable std::map<ContextID, std::recursive_mutex> context_locks_;

    std::recursive_mutex& get_lock(const ContextID context) const;

    void Cleanup();
    void Init();
    void storage_gc_hook() override;

    Manager(
        const Flag& running,
        const ArgList& args,
        const api::Settings& config,
        const api::Crypto& crypto,
        const opentxs::network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);
    Manager() = delete;
    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager& operator=(Manager&&) = delete;
};
}  // namespace opentxs::api::client::implementation
