// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::server::implementation
{
class Wallet final : public api::implementation::Wallet
{
public:
    std::shared_ptr<const opentxs::ClientContext> ClientContext(
        const identifier::Nym& remoteNymID) const final;
    std::shared_ptr<const opentxs::Context> Context(
        const identifier::Server& notaryID,
        const identifier::Nym& clientNymID) const final;
    Editor<opentxs::ClientContext> mutable_ClientContext(
        const identifier::Nym& remoteNymID,
        const PasswordPrompt& reason) const final;
    Editor<opentxs::Context> mutable_Context(
        const identifier::Server& notaryID,
        const identifier::Nym& clientNymID,
        const PasswordPrompt& reason) const final;

    ~Wallet() = default;

private:
    friend opentxs::Factory;

    using ot_super = api::implementation::Wallet;

    const api::server::internal::Manager& server_;

    void instantiate_client_context(
        const proto::Context& serialized,
        const Nym_p& localNym,
        const Nym_p& remoteNym,
        std::shared_ptr<opentxs::internal::Context>& output) const final;
    bool load_legacy_account(
        const Identifier& accountID,
        const eLock& lock,
        AccountLock& row) const final;
    Nym_p signer_nym(const identifier::Nym& id) const final;

    Wallet(const api::server::internal::Manager& server);
    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    Wallet& operator=(const Wallet&) = delete;
    Wallet& operator=(Wallet&&) = delete;
};
}  // namespace opentxs::api::server::implementation
