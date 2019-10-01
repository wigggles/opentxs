// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class Manager final : opentxs::api::client::internal::Manager,
                      api::implementation::Core
{
public:
    const api::client::Activity& Activity() const final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const api::client::Blockchain& Blockchain() const final;
#endif
    const api::client::Contacts& Contacts() const final;
    const OTAPI_Exec& Exec(const std::string& wallet = "") const final;
    using Core::Lock;
    std::recursive_mutex& Lock(
        const identifier::Nym& nymID,
        const identifier::Server& serverID) const final;
    const OT_API& OTAPI(const std::string& wallet = "") const final;
    const client::OTX& OTX() const final;
    const api::client::Pair& Pair() const final;
    const client::ServerAction& ServerAction() const final;
    const api::client::UI& UI() const final;
    const client::Workflow& Workflow() const final;
    const api::network::ZMQ& ZMQ() const final;

    void StartActivity(const PasswordPrompt& reason) final;
    void StartContacts(const PasswordPrompt& reason) final;

    ~Manager() final;

private:
    friend opentxs::Factory;

    std::unique_ptr<api::network::ZMQ> zeromq_;
    std::unique_ptr<api::client::internal::Contacts> contacts_;
    std::unique_ptr<api::client::internal::Activity> activity_;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    std::unique_ptr<api::client::Blockchain> blockchain_;
#endif
    std::unique_ptr<api::client::Workflow> workflow_;
    std::unique_ptr<OT_API> ot_api_;
    std::unique_ptr<OTAPI_Exec> otapi_exec_;
    std::unique_ptr<api::client::ServerAction> server_action_;
    std::unique_ptr<api::client::OTX> otx_;
    std::unique_ptr<api::client::internal::Pair> pair_;
    std::unique_ptr<api::client::UI> ui_;
    mutable std::mutex map_lock_;
    mutable std::map<ContextID, std::recursive_mutex> context_locks_;

    std::recursive_mutex& get_lock(const ContextID context) const;

    void Cleanup();
    void Init(const PasswordPrompt& reason);

    Manager(
        const api::internal::Context& parent,
        Flag& running,
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
