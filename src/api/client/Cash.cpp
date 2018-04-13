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

#include "opentxs/client/SwigWrap.hpp"

#include "opentxs/cash/Purse.hpp"

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

bool Cash::withdraw_and_export_cash(
    const std::string& ACCT_ID,
    const std::string& RECIPIENT_NYM_ID,
    std::int64_t AMOUNT,
    std::shared_ptr<const Purse>& recipientCopy,
    std::shared_ptr<const Purse>& senderCopy,
    bool bPasswordProtected/*=false*/) const
{
    rLock lock(api_lock_);

    const std::string server = SwigWrap::GetAccountWallet_NotaryID(ACCT_ID);
    const std::string mynym = SwigWrap::GetAccountWallet_NymID(ACCT_ID);
    const std::string asset_type =
        SwigWrap::GetAccountWallet_InstrumentDefinitionID(ACCT_ID);
    std::string hisnym = RECIPIENT_NYM_ID;
    std::string indices;

    return 1 == withdraw_and_export_cash_low_level(
                             server,
                             mynym,
                             asset_type,
                             ACCT_ID,
                             hisnym,
                             std::to_string(AMOUNT),
                             indices,
                             bPasswordProtected,
                             recipientCopy,
                             senderCopy);
}

int32_t Cash::withdraw_and_export_cash_low_level(
    const std::string& server,
    const std::string& mynym,
    const std::string& assetType,
    const std::string& myacct,
    std::string& hisnym,
    const std::string& amount,
    std::string& indices,
    bool hasPassword,
    std::shared_ptr<const Purse>& recipientCopy,
    std::shared_ptr<const Purse>& senderCopy) const
{
    int64_t startAmount = "" == amount ? 0 : stoll(amount);

    // What we want to do from here is, see if we can send the cash purely using
    // cash we already have in the local purse. If so, we just package it up and
    // send it off using send_user_payment.
    //
    // But if we do NOT have the proper cash tokens in the local purse to send,
    // then we need to withdraw enough tokens until we do, and then try sending
    // again.

    int64_t remain = startAmount;
    if (!get_purse_indices_or_amount(server, mynym, assetType, remain, indices)) {
        if (!indices.empty()) {
            otOut << "Error: invalid purse indices.\n";
            return -1;
        }

        // Not enough cash found in existing purse to match the amount
        CmdWithdrawCash cmd;
        if (1 != cmd.withdrawCash(myacct, remain)) {
            otOut << "Error: cannot withdraw cash.\n";
            return -1;
        }

        remain = startAmount;
        if (!get_purse_indices_or_amount(
                server, mynym, assetType, remain, indices)) {
            otOut << "Error: cannot retrieve purse indices. "
            "(It's possible that you have enough cash, yet lack the correct "
            "denominations of token for the exact amount requested).\n";
            return -1;
        }
    }

    CmdExportCash cmd;
    std::string retainedCopy = "";
    std::string exportedCash = cmd.exportCash(
        server, mynym, assetType, hisnym, indices, hasPassword, retainedCopy);
    if (exportedCash.empty()) {
        otOut << "Error: cannot export cash.\n";
        return -1;
    }
    // By this point, exportedCash and retainedCopy should both be valid.

    std::shared_ptr<const Purse> pRecipientCopy(
        Purse::PurseFactory(String(exportedCash)));
    std::shared_ptr<const Purse> pSenderCopy(
        Purse::PurseFactory(String(retainedCopy)));

    OT_ASSERT(pRecipientCopy);
    OT_ASSERT(pSenderCopy);

    recipientCopy = pRecipientCopy;
    senderCopy    = pSenderCopy;

    return 1;
}

// If you pass the indices, this function returns true if those exact indices
// exist. In that case, this function will also set remain to the total.
//
// If, instead, you pass remain and a blank indices, this function will try to
// determine the indices that would create remain, if they were selected.

bool Cash::get_purse_indices_or_amount(
    const std::string& server,
    const std::string& mynym,
    const std::string& assetType,
    std::int64_t& remain,
    std::string& indices) const
{
    bool findAmountFromIndices = "" != indices && 0 == remain;
    bool findIndicesFromAmount = "" == indices && 0 != remain;
    if (!findAmountFromIndices && !findIndicesFromAmount) {
        otOut << "Error: invalid parameter combination.\n";
        return false;
    }

    std::string purse = SwigWrap::LoadPurse(server, assetType, mynym);
    if ("" == purse) {
        otOut << "Error: cannot load purse.\n";
        return false;
    }

    std::int32_t items = SwigWrap::Purse_Count(server, assetType, purse);
    if (0 > items) {
        otOut << "Error: cannot load purse item count.\n\n";
        return false;
    }

    if (0 == items) {
        otOut << "Error: the purse is empty.\n\n";
        return false;
    }

    for (std::int32_t i = 0; i < items; i++) {
        std::string token = SwigWrap::Purse_Peek(server, assetType, mynym, purse);
        if ("" == token) {
            otOut << "Error:cannot load token from purse.\n";
            return false;
        }

        purse = SwigWrap::Purse_Pop(server, assetType, mynym, purse);
        if ("" == purse) {
            otOut << "Error: cannot load updated purse.\n";
            return false;
        }

        std::int64_t denomination =
            SwigWrap::Token_GetDenomination(server, assetType, token);
        if (0 >= denomination) {
            otOut << "Error: cannot get token denomination.\n";
            return false;
        }

        time64_t validTo = SwigWrap::Token_GetValidTo(server, assetType, token);
        if (OT_TIME_ZERO > validTo) {
            otOut << "Error: cannot get token validTo.\n";
            return false;
        }

        time64_t time = SwigWrap::GetTime();
        if (OT_TIME_ZERO > time) {
            otOut << "Error: cannot get token time.\n";
            return false;
        }

        if (time > validTo) {
            otOut << "Skipping: token is expired.\n";
            continue;
        }

        if (findAmountFromIndices) {
            if ("all" == indices ||
                SwigWrap::NumList_VerifyQuery(indices, std::to_string(i))) {
                remain += denomination;
            }
            continue;
        }

        // TODO: There could be a denomination order that will cause this
        // function to fail, even though there is a denomination combination
        // that would make it succeeed. Example: try to find 6 when the
        // denominations are: 5, 2, 2, and 2. This will not succeed since it
        // will use the 5 first and then cannot satisfy the remaining 1 even
        // though the three 2's would satisfy the 6...

        if (denomination <= remain) {
            indices = SwigWrap::NumList_Add(indices, std::to_string(i));
            remain -= denomination;
            if (0 == remain) {
                return true;
            }
        }
    }

    return findAmountFromIndices ? true : false;
}


bool Cash::withdraw_and_send_cash(
    const std::string& ACCT_ID,
    const std::string& recipientNymID,
    std::int64_t AMOUNT) const
{
    rLock lock(api_lock_);

    CmdSendCash sendCash;
    return 1 == sendCash.run(
                    "", // server
                    "", // mynym
                    ACCT_ID, //myacct
                    "", //mypurse
                    recipientNymID, //hisnym
                    std::to_string(AMOUNT), // amount
                    "", // indices
                    ""); // password.
}

#endif
}  // namespace opentxs
