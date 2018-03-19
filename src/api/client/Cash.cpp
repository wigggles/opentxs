/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/client/commands/CmdDeposit.hpp"
#include "opentxs/client/commands/CmdWithdrawCash.hpp"
#include "opentxs/client/commands/CmdExportCash.hpp"
#include "opentxs/client/commands/CmdSendCash.hpp"

#include "Cash.hpp"

//#define OT_METHOD "opentxs::api::client::implementation::Cash::"

namespace opentxs::api::client::implementation
{
Cash::Cash(std::recursive_mutex& apilock)
    : api_lock_(apilock)
{
}

#if OT_CASH
bool Cash::deposit_cash(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    const std::string& STR_PURSE) const
{
    rLock lock(api_lock_);

    CmdDeposit cmd;
    return 1 == cmd.depositPurse(notaryID, ACCT_ID, nymID, STR_PURSE, "");
}

bool Cash::deposit_local_purse(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    const std::string& STR_INDICES) const
{
    rLock lock(api_lock_);

    CmdDeposit cmd;
    return 1 == cmd.depositPurse(notaryID, ACCT_ID, nymID, "", STR_INDICES);
}

bool Cash::easy_withdraw_cash(const std::string& ACCT_ID, std::int64_t AMOUNT)
    const
{
    rLock lock(api_lock_);

    CmdWithdrawCash cmd;
    return 1 == cmd.withdrawCash(ACCT_ID, AMOUNT);
}

std::string Cash::export_cash(
    const std::string& notaryID,
    const std::string& FROM_nymID,
    const std::string& instrumentDefinitionID,
    const std::string& TO_nymID,
    const std::string& STR_INDICES,
    bool bPasswordProtected,
    std::string& STR_RETAINED_COPY) const
{
    rLock lock(api_lock_);

    std::string to_nym_id = TO_nymID;
    CmdExportCash cmd;
    return cmd.exportCash(
        notaryID,
        FROM_nymID,
        instrumentDefinitionID,
        to_nym_id,
        STR_INDICES,
        bPasswordProtected,
        STR_RETAINED_COPY);
}

bool Cash::withdraw_and_send_cash(
    const std::string& ACCT_ID,
    const std::string& recipientNymID,
    std::int64_t AMOUNT) const
{
    rLock lock(api_lock_);

    CmdSendCash sendCash;
    return 1 == sendCash.run(
                    "",
                    "",
                    ACCT_ID,
                    "",
                    recipientNymID,
                    std::to_string(AMOUNT),
                    "",
                    "");
}
#endif
}  // namespace opentxs
