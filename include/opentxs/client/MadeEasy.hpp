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

#ifndef OPENTXS_CLIENT_MADEEASY_HPP
#define OPENTXS_CLIENT_MADEEASY_HPP

#include "opentxs/core/util/Common.hpp"

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace opentxs
{

class Api;

class MadeEasy
{
private:
    friend class Api;

    std::recursive_mutex& lock_;

    MadeEasy(std::recursive_mutex& lock);
    MadeEasy() = delete;
    MadeEasy(const MadeEasy&) = delete;
    MadeEasy(const MadeEasy&&) = delete;
    MadeEasy& operator=(const MadeEasy&) = delete;
    MadeEasy& operator=(const MadeEasy&&) = delete;

public:
    EXPORT std::string ping_notary(
        const std::string& NOTARY_ID, const std::string& NYM_ID);
    EXPORT std::string check_nym(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& TARGET_NYM_ID);
    EXPORT std::string create_asset_acct(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& ACCOUNT_ID);
    EXPORT std::string unregister_account(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& ACCOUNT_ID);
    EXPORT std::string unregister_nym(
        const std::string& NOTARY_ID, const std::string& NYM_ID);
    EXPORT std::string deposit_payment_plan(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& THE_PAYMENT_PLAN);
    EXPORT std::string deposit_purse(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& ACCT_ID, const std::string& STR_PURSE);
    EXPORT int32_t depositCashPurse(
        const std::string& notaryID, const std::string& instrumentDefinitionID,
        const std::string& nymID, const std::string& oldPurse,
        const std::vector<std::string>& selectedTokens,
        const std::string& accountID,
        bool bReimportIfFailure, // So we don't re-import a purse that wasn't internal to begin with.
        std::string * pOptionalOutput=nullptr);
    EXPORT std::string exchange_basket_currency(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& ASSET_TYPE, const std::string& THE_BASKET,
        const std::string& ACCT_ID, bool IN_OR_OUT);
    EXPORT bool exchangeCashPurse(
        const std::string& notaryID, const std::string& instrumentDefinitionID,
        const std::string& nymID, std::string& oldPurse,
        const std::vector<std::string>& selectedTokens);
    EXPORT std::string exportCashPurse(
        const std::string& notaryID, const std::string& instrumentDefinitionID,
        const std::string& nymID, const std::string& oldPurse,
        const std::vector<std::string>& selectedTokens,
        std::string& recipientNymID, bool bPasswordProtected,
        std::string& strRetainedCopy);
    EXPORT bool importCashPurse(
        const std::string& notaryID, const std::string& nymID,
        const std::string& instrumentDefinitionID, std::string& userInput,
        bool isPurse);
    EXPORT std::string issue_asset_type(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& THE_CONTRACT);
    EXPORT std::string issue_basket_currency(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& THE_BASKET);
    EXPORT std::string load_or_retrieve_contract(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& CONTRACT_ID);
    EXPORT std::string load_or_retrieve_encrypt_key(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& TARGET_NYM_ID);
    EXPORT std::string load_or_retrieve_mint(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& INSTRUMENT_DEFINITION_ID);
    EXPORT std::string load_public_encryption_key(
        const std::string& NYM_ID); // from local storage.
    EXPORT std::string load_public_signing_key(
        const std::string& NYM_ID); // from local storage.
    EXPORT std::string process_inbox(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& ACCOUNT_ID, const std::string& RESPONSE_LEDGER);
    EXPORT bool processCashPurse(
        std::string& newPurse, std::string& newPurseForSender,
        const std::string& notaryID, const std::string& instrumentDefinitionID,
        const std::string& nymID, std::string& oldPurse,
        const std::vector<std::string>& selectedTokens,
        const std::string& recipientNymID, bool bPWProtectOldPurse,
        bool bPWProtectNewPurse);
    EXPORT bool retrieve_account(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        bool bForceDownload); // bForceDownload=false
    EXPORT std::string retrieve_contract(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& CONTRACT_ID);
    EXPORT std::string retrieve_mint(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& INSTRUMENT_DEFINITION_ID);
    EXPORT int32_t retrieve_nym(
        const std::string& strNotaryID, const std::string& strMyNymID,
        bool& bWasMsgSent, bool bForceDownload);
    EXPORT std::string send_transfer(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& ACCT_FROM, const std::string& ACCT_TO,
        int64_t AMOUNT, const std::string& NOTE);
    EXPORT std::string send_user_cash_pubkey(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& RECIPIENT_NYM_ID,
        const std::string& RECIPIENT_PUBKEY, const std::string& THE_INSTRUMENT,
        const std::string& INSTRUMENT_FOR_SENDER);
    EXPORT std::string send_user_msg(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& RECIPIENT_NYM_ID, const std::string& THE_MESSAGE);
    EXPORT std::string send_user_msg_pubkey(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& RECIPIENT_NYM_ID,
        const std::string& RECIPIENT_PUBKEY, const std::string& THE_MESSAGE);
    EXPORT std::string send_user_pmnt_pubkey(
        const std::string& NOTARY_ID, const std::string& NYM_ID,
        const std::string& RECIPIENT_NYM_ID,
        const std::string& RECIPIENT_PUBKEY, const std::string& THE_INSTRUMENT);
    EXPORT std::string stat_asset_account(
        const std::string& ACCOUNT_ID);

    ~MadeEasy() = default;
};

} // namespace opentxs

#endif // OPENTXS_CLIENT_MADEEASY_HPP
