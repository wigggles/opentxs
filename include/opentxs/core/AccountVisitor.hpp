// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_ACCOUNTVISITOR_HPP
#define OPENTXS_CORE_ACCOUNTVISITOR_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/identifier/Server.hpp"

#include <map>
#include <string>

namespace opentxs
{
class AccountVisitor
{
public:
    using mapOfAccounts = std::map<std::string, const Account*>;

    const identifier::Server& GetNotaryID() const { return notaryID_; }

    virtual bool Trigger(
        const Account& account,
        const PasswordPrompt& reason) = 0;

    const api::Wallet& Wallet() const { return wallet_; }

    virtual ~AccountVisitor() = default;

protected:
    const api::Wallet& wallet_;
    const OTServerID notaryID_;
    mapOfAccounts* loadedAccounts_;

    AccountVisitor(
        const api::Wallet& wallet,
        const identifier::Server& notaryID);

private:
    AccountVisitor() = delete;
    AccountVisitor(const AccountVisitor&) = delete;
    AccountVisitor(AccountVisitor&&) = delete;
    AccountVisitor& operator=(const AccountVisitor&) = delete;
    AccountVisitor& operator=(AccountVisitor&&) = delete;
};
}  // namespace opentxs
#endif
