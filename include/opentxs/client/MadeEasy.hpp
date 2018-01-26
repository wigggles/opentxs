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

#include "opentxs/Version.hpp"

#include "opentxs/Types.hpp"

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace opentxs
{
class OT_API;
class OTAPI_Exec;

namespace api
{
namespace client
{
class Wallet;
}  // namespace client

namespace implementation
{
class Api;
}  // namespace implementation
}  // namespace api

class MadeEasy
{
public:
    EXPORT std::string check_nym(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& TARGET_NYM_ID) const;
    EXPORT std::string create_asset_acct(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID) const;
    EXPORT std::string unregister_account(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID) const;
    EXPORT std::string unregister_nym(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID) const;
    EXPORT std::string deposit_payment_plan(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& THE_PAYMENT_PLAN) const;
#if OT_CASH
    EXPORT std::string deposit_purse(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCT_ID,
        const std::string& STR_PURSE) const;
    EXPORT std::int32_t depositCashPurse(
        const std::string& notaryID,
        const std::string& instrumentDefinitionID,
        const std::string& nymID,
        const std::string& oldPurse,
        const std::vector<std::string>& selectedTokens,
        const std::string& accountID,
        bool bReimportIfFailure,  // So we don't re-import a purse that wasn't
                                  // internal to begin with.
        std::string* pOptionalOutput = nullptr) const;
#endif  // OT_CASH
    EXPORT std::string exchange_basket_currency(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ASSET_TYPE,
        const std::string& THE_BASKET,
        const std::string& ACCT_ID,
        bool IN_OR_OUT) const;
#if OT_CASH
    EXPORT bool exchangeCashPurse(
        const std::string& notaryID,
        const std::string& instrumentDefinitionID,
        const std::string& nymID,
        std::string& oldPurse,
        const std::vector<std::string>& selectedTokens) const;
    EXPORT std::string exportCashPurse(
        const std::string& notaryID,
        const std::string& instrumentDefinitionID,
        const std::string& nymID,
        const std::string& oldPurse,
        const std::vector<std::string>& selectedTokens,
        std::string& recipientNymID,
        bool bPasswordProtected,
        std::string& strRetainedCopy) const;
    EXPORT bool importCashPurse(
        const std::string& notaryID,
        const std::string& nymID,
        const std::string& instrumentDefinitionID,
        std::string& userInput,
        bool isPurse) const;
#endif  // OT_CASH
    EXPORT std::string issue_asset_type(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& THE_CONTRACT) const;
    EXPORT std::string issue_basket_currency(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& THE_BASKET) const;
    EXPORT std::string load_or_retrieve_contract(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& CONTRACT_ID) const;
    EXPORT std::string load_or_retrieve_encrypt_key(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& TARGET_NYM_ID) const;
#if OT_CASH
    EXPORT std::string load_or_retrieve_mint(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& INSTRUMENT_DEFINITION_ID) const;
#endif  // OT_CASH
    EXPORT std::string load_public_encryption_key(
        const std::string& NYM_ID) const;  // from local storage.
    EXPORT std::string load_public_signing_key(
        const std::string& NYM_ID) const;  // from local storage.
    EXPORT std::string process_inbox(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        const std::string& RESPONSE_LEDGER) const;
#if OT_CASH
    EXPORT bool processCashPurse(
        std::string& newPurse,
        std::string& newPurseForSender,
        const std::string& notaryID,
        const std::string& instrumentDefinitionID,
        const std::string& nymID,
        std::string& oldPurse,
        const std::vector<std::string>& selectedTokens,
        const std::string& recipientNymID,
        bool bPWProtectOldPurse,
        bool bPWProtectNewPurse) const;
#endif  // OT_CASH
    EXPORT bool retrieve_account(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCOUNT_ID,
        bool bForceDownload) const;  // bForceDownload=false
    EXPORT std::string retrieve_contract(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& CONTRACT_ID) const;
#if OT_CASH
    EXPORT std::string retrieve_mint(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& INSTRUMENT_DEFINITION_ID) const;
#endif  // OT_CASH
    EXPORT std::int32_t retrieve_nym(
        const std::string& strNotaryID,
        const std::string& strMyNymID,
        bool& bWasMsgSent,
        bool bForceDownload) const;
    EXPORT std::string send_transfer(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCT_FROM,
        const std::string& ACCT_TO,
        const std::int64_t AMOUNT,
        const std::string& NOTE,
        TransactionNumber& number) const;
#if OT_CASH
    EXPORT std::string send_user_cash_pubkey(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& RECIPIENT_NYM_ID,
        const std::string& RECIPIENT_PUBKEY,
        const std::string& THE_INSTRUMENT,
        const std::string& INSTRUMENT_FOR_SENDER) const;
#endif  // OT_CASH
    EXPORT std::string send_user_msg(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& RECIPIENT_NYM_ID,
        const std::string& THE_MESSAGE) const;
    EXPORT std::string send_user_msg_pubkey(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& RECIPIENT_NYM_ID,
        const std::string& RECIPIENT_PUBKEY,
        const std::string& THE_MESSAGE) const;
    EXPORT std::string send_user_pmnt_pubkey(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& RECIPIENT_NYM_ID,
        const std::string& RECIPIENT_PUBKEY,
        const std::string& THE_INSTRUMENT) const;
    EXPORT std::string stat_asset_account(const std::string& ACCOUNT_ID) const;

    ~MadeEasy() = default;

private:
    friend class api::implementation::Api;

    std::recursive_mutex& lock_;
    OTAPI_Exec& exec_;
    OT_API& otapi_;
    api::client::Wallet& wallet_;

    MadeEasy(
        std::recursive_mutex& lock,
        OTAPI_Exec& exec,
        OT_API& otapi,
        api::client::Wallet& wallet);
    MadeEasy() = delete;
    MadeEasy(const MadeEasy&) = delete;
    MadeEasy(const MadeEasy&&) = delete;
    MadeEasy& operator=(const MadeEasy&) = delete;
    MadeEasy& operator=(const MadeEasy&&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CLIENT_MADEEASY_HPP
