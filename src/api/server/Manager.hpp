// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef IMPLEMENTATION_OPENTXS_API_SERVER_MANAGER_HPP
#define IMPLEMENTATION_OPENTXS_API_SERVER_MANAGER_HPP

#include "Internal.hpp"

namespace opentxs::api::server::implementation
{
class Manager final : opentxs::api::server::Manager,
                      opentxs::api::implementation::Scheduler
{
public:
    const api::Settings& Config() const override { return config_; }
    const api::Crypto& Crypto() const override { return crypto_; }
    const api::network::Dht& DHT() const override;
    const api::Factory& Factory() const override;
    const std::string GetCommandPort() const override;
    const std::string GetDefaultBindIP() const override;
    const std::string GetEEP() const override;
    const std::string GetExternalIP() const override;
    const std::string GetListenCommand() const override;
    const std::string GetListenNotify() const override;
    const std::string GetOnion() const override;
#if OT_CASH
    std::shared_ptr<Mint> GetPrivateMint(
        const Identifier& unitID,
        std::uint32_t series) const override;
    std::shared_ptr<const Mint> GetPublicMint(
        const Identifier& unitID) const override;
#endif  // OT_CASH
    const std::string GetUserName() const override;
    const std::string GetUserTerms() const override;
    const Identifier& ID() const override;
    int Instance() const override { return instance_; }
    const Identifier& NymID() const override;
#if OT_CASH
    void ScanMints() const override;
#endif  // OT_CASH
    void Schedule(
        const std::chrono::seconds& interval,
        const PeriodicTask& task,
        const std::chrono::seconds& last =
            std::chrono::seconds(0)) const override
    {
        Scheduler::Schedule(interval, task, last);
    }
#if OT_CRYPTO_WITH_BIP39
    const api::HDSeed& Seeds() const override { return seeds_; }
#if OT_CASH
#endif
    const api::storage::Storage& Storage() const override { return storage_; }
    void UpdateMint(const Identifier& unitID) const override;
#endif  // OT_CASH
    const api::Wallet& Wallet() const override;
    const opentxs::network::zeromq::Context& ZeroMQ() const override
    {
        return zmq_context_;
    }

    ~Manager();

private:
    friend opentxs::Factory;

#if OT_CASH
    typedef std::map<std::string, std::shared_ptr<Mint>> MintSeries;
#endif  // OT_CASH

    const ArgList& args_;
    const api::storage::Storage& storage_;
    const api::Legacy& legacy_;
    const api::Settings& config_;
    const api::Crypto& crypto_;
#if OT_CRYPTO_WITH_BIP39
    const api::HDSeed& seeds_;
#endif
    const opentxs::network::zeromq::Context& zmq_context_;
    const int instance_{0};
    const Flag& running_;
    std::unique_ptr<api::Factory> factory_;
    std::unique_ptr<api::Wallet> wallet_;
    std::unique_ptr<api::network::Dht> dht_;
    std::unique_ptr<opentxs::server::Server> server_p_;
    opentxs::server::Server& server_;
    std::unique_ptr<opentxs::server::MessageProcessor> message_processor_p_;
    opentxs::server::MessageProcessor& message_processor_;
#if OT_CASH
    std::unique_ptr<std::thread> mint_thread_;
    mutable std::mutex mint_lock_;
    mutable std::mutex mint_update_lock_;
    mutable std::mutex mint_scan_lock_;
    mutable std::map<std::string, MintSeries> mints_;
    mutable std::deque<std::string> mints_to_check_;
#endif  // OT_CASH

#if OT_CASH
    void generate_mint(
        const std::string& serverID,
        const std::string& unitID,
        const std::uint32_t series) const;
#endif  // OT_CASH
    const std::string get_arg(const std::string& argName) const;
#if OT_CASH
    std::int32_t last_generated_series(
        const std::string& serverID,
        const std::string& unitID) const;
    std::shared_ptr<Mint> load_private_mint(
        const Lock& lock,
        const std::string& unitID,
        const std::string seriesID) const;
    std::shared_ptr<Mint> load_public_mint(
        const Lock& lock,
        const std::string& unitID,
        const std::string seriesID) const;
    void mint() const;
#endif  // OT_CASH
    bool verify_lock(const Lock& lock, const std::mutex& mutex) const;
#if OT_CASH
    std::shared_ptr<Mint> verify_mint(
        const Lock& lock,
        const std::string& unitID,
        const std::string seriesID,
        std::shared_ptr<Mint>& mint) const;
    bool verify_mint_directory(const std::string& serverID) const;
#endif  // OT_CASH

    void Cleanup();
    void Init();
    void Start() override;
    void storage_gc_hook() override;

    Manager(
        const ArgList& args,
        const api::storage::Storage& storage,
        const api::Crypto& crypto,
#if OT_CRYPTO_WITH_BIP39
        const api::HDSeed& seeds,
#endif
        const api::Legacy& legacy,
        const api::Settings& config,
        const opentxs::network::zeromq::Context& context,
        const Flag& running,
        const int instance);
    Manager() = delete;
    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager& operator=(Manager&&) = delete;
};
}  // namespace opentxs::api::server::implementation
#endif  // IMPLEMENTATION_OPENTXS_API_SERVER_MANAGER_HPP
