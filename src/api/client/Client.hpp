// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef SRC_API_CLIENT_CLIENT_HPP
#define SRC_API_CLIENT_CLIENT_HPP

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class Client : virtual public opentxs::api::client::Client
{
public:
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const api::client::Blockchain& Blockchain() const override;
#endif
    const api::client::Cash& Cash() const override;
    const OTAPI_Exec& Exec(const std::string& wallet = "") const override;
    std::recursive_mutex& Lock(
        const Identifier& nymID,
        const Identifier& serverID) const override;
    const OT_API& OTAPI(const std::string& wallet = "") const override;
    const api::client::Pair& Pair() const override;
    const client::ServerAction& ServerAction() const override;
    const client::Sync& Sync() const override;
    const api::client::UI& UI() const override;
    const client::Workflow& Workflow() const override;

    ~Client();

private:
    friend Factory;

    const Flag& running_;
    const api::client::Wallet& wallet_;
    const api::network::ZMQ& zmq_;
    const api::storage::Storage& storage_;
    const api::Activity& activity_;
    const api::ContactManager& contacts_;
    const api::Crypto& crypto_;
    const api::Identity& identity_;
    const api::Legacy& legacy_;
    const api::Settings& config_;

#if OT_CRYPTO_SUPPORTED_KEY_HD
    std::unique_ptr<api::client::Blockchain> blockchain_;
#endif
    std::unique_ptr<api::client::Cash> cash_;
    std::unique_ptr<api::client::Pair> pair_;
    std::unique_ptr<api::client::ServerAction> server_action_;
    std::unique_ptr<api::client::Sync> sync_;
    std::unique_ptr<api::client::UI> ui_;
    std::unique_ptr<api::client::Workflow> workflow_;
    std::unique_ptr<OT_API> ot_api_;
    std::unique_ptr<OTAPI_Exec> otapi_exec_;

    mutable std::recursive_mutex lock_;
    mutable std::mutex map_lock_;
    mutable std::map<ContextID, std::recursive_mutex> context_locks_;

    std::recursive_mutex& get_lock(const ContextID context) const;

    void Cleanup();
    void Init();
#if OT_CRYPTO_SUPPORTED_KEY_HD
    void Init_Blockchain();
#endif
    void Init_UI();

    Client(
        const Flag& running,
        const api::Activity& activity,
        const api::Settings& config,
        const api::ContactManager& contacts,
        const api::Crypto& crypto,
        const api::Identity& identity,
        const api::Legacy& legacy,
        const api::storage::Storage& storage,
        const api::client::Wallet& wallet,
        const api::network::ZMQ& zmq);
    Client() = delete;
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&) = delete;
};
}  // namespace opentxs::api::client::implementation
#endif  // SRC_API_CLIENT_CLIENT_HPP
