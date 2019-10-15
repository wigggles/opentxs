// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/AccountList.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <string>

namespace opentxs
{
namespace server
{
class Transactor
{
    friend class MainFile;

public:
    Transactor(Server& server, const PasswordPrompt& reason);
    ~Transactor() = default;

    bool issueNextTransactionNumber(TransactionNumber& txNumber);
    bool issueNextTransactionNumberToNym(
        ClientContext& context,
        TransactionNumber& txNumber);

    TransactionNumber transactionNumber() const { return transactionNumber_; }

    void transactionNumber(TransactionNumber value)
    {
        transactionNumber_ = value;
    }

    bool addBasketAccountID(
        const Identifier& basketId,
        const Identifier& basketAccountId,
        const Identifier& basketContractId);
    bool lookupBasketAccountID(
        const Identifier& basketId,
        Identifier& basketAccountId);

    bool lookupBasketAccountIDByContractID(
        const Identifier& basketContractId,
        Identifier& basketAccountId);
    bool lookupBasketContractIDByAccountID(
        const Identifier& basketAccountId,
        Identifier& basketContractId);

    // Whenever the server issues a voucher (like a cashier's cheque), it puts
    // the funds in one of these voucher accounts (one for each instrument
    // definition ID). Then it issues the cheque from the same account.
    // TODO: also should save the cheque itself to a folder, where the folder is
    // named based on the date that the cheque will expire.  This way, the
    // server operator can go back later, or have a script, to retrieve the
    // cheques from the expired folders, and total them. The server operator is
    // free to remove that total from the Voucher Account once the cheque has
    // expired: it is his money now.
    ExclusiveAccount getVoucherAccount(
        const identifier::UnitDefinition& instrumentDefinitionID);

private:
    typedef std::map<std::string, std::string> BasketsMap;

    Server& server_;
    const PasswordPrompt& reason_;
    // This stores the last VALID AND ISSUED transaction number.
    TransactionNumber transactionNumber_;
    // maps basketId with basketAccountId
    BasketsMap idToBasketMap_;
    // basket issuer account ID, which is *different* on each server, using the
    // Basket Currency's ID, which is the *same* on every server.)
    // Need a way to look up a Basket Account ID using its Contract ID
    BasketsMap contractIdToBasketAccountId_;
    // The list of voucher accounts (see GetVoucherAccount below for details)
    AccountList voucherAccounts_;

    Transactor() = delete;
};
}  // namespace server
}  // namespace opentxs
