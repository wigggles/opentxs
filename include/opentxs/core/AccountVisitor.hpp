// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_ACCOUNTVISITOR_HPP
#define OPENTXS_CORE_ACCOUNTVISITOR_HPP

#include "opentxs/Forward.hpp"

#include <map>
#include <string>

namespace opentxs
{
class AccountVisitor
{
public:
    using mapOfAccounts = std::map<std::string, const Account*>;

    OTIdentifier GetNotaryID() const { return notaryID_; }

    virtual bool Trigger(const Account& account) = 0;

    const api::Wallet& Wallet() const { return wallet_; }

    virtual ~AccountVisitor() = default;

protected:
    const api::Wallet& wallet_;
    const OTIdentifier notaryID_;
    mapOfAccounts* loadedAccounts_;

    AccountVisitor(const api::Wallet& wallet, const Identifier& notaryID);

private:
    AccountVisitor() = delete;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_ACCOUNTVISITOR_HPP
