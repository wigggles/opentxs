// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class Cash : virtual public client::Cash, Lockable
{
public:
#if OT_CASH
    bool deposit_cash(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCT_ID,
        const std::string& STR_PURSE) const override;
    bool deposit_local_purse(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCT_ID,
        const std::string& STR_INDICES) const override;
    std::int32_t deposit_purse(
        const std::string& server,
        const std::string& myacct,
        const std::string& mynym,
        std::string instrument,
        const std::string& indices,
        std::string* pOptionalOutput = nullptr) const override;  // server
                                                                 // reply.
    bool easy_withdraw_cash(const std::string& ACCT_ID, std::int64_t AMOUNT)
        const override;
    std::string export_cash(
        const std::string& NOTARY_ID,
        const std::string& FROM_NYM_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& TO_NYM_ID,
        const std::string& STR_INDICES,
        bool bPasswordProtected,
        std::string& STR_RETAINED_COPY) const override;
    std::int32_t send_cash(
        std::string& response,
        const std::string& server,
        const std::string& mynym,
        const std::string& assetType,
        const std::string& myacct,
        std::string& hisnym,
        const std::string& amount,
        std::string& indices,
        bool hasPassword) const override;
    bool withdraw_and_export_cash(
        const std::string& ACCT_ID,
        const std::string& RECIPIENT_NYM_ID,
        std::int64_t AMOUNT,
        std::shared_ptr<const Purse>& recipientCopy,
        std::shared_ptr<const Purse>& senderCopy,
        bool bPasswordProtected = false) const override;
    bool withdraw_and_send_cash(
        const std::string& ACCT_ID,
        const std::string& RECIPIENT_NYM_ID,
        std::int64_t AMOUNT) const override;

#endif  // OT_CASH

protected:
#if OT_CASH

    std::int32_t deposit_purse_low_level(
        const std::string& notaryID,
        const std::string& unitTypeID,
        const std::string& nymID,
        const std::string& oldPurse,
        const std::vector<std::string>& selectedTokens,
        const std::string& accountID,
        const bool bReimportIfFailure,  // So we don't re-import a purse that
                                        // wasn't internal to begin with.
        std::string* pOptionalOutput = nullptr) const;  // server reply.

    std::string export_cash_low_level(
        const std::string& notaryID,
        const std::string& instrumentDefinitionID,
        const std::string& nymID,
        const std::string& oldPurse,
        const std::vector<std::string>& selectedTokens,
        std::string& recipientNymID,
        bool bPasswordProtected,
        std::string& strRetainedCopy) const;

    std::int32_t easy_withdraw_cash_low_level(
        const std::string& myacct,
        std::int64_t amount) const;

    std::string check_nym(
        const std::string& notaryID,
        const std::string& nymID,
        const std::string& targetNymID) const;

    bool get_purse_indices_or_amount(
        const std::string& server,
        const std::string& mynym,
        const std::string& assetType,
        std::int64_t& remain,
        std::string& indices) const;

    bool get_tokens(
        std::vector<std::string>& tokens,
        const std::string& server,
        const std::string& mynym,
        const std::string& assetType,
        std::string purse,
        const std::string& indices) const;

    std::string load_or_retrieve_mint(
        const std::string& notaryID,
        const std::string& nymID,
        const std::string& unitTypeID) const;

    bool process_cash_purse(
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

    std::int32_t withdraw_and_export_cash_low_level(
        const std::string& server,
        const std::string& mynym,
        const std::string& assetType,
        const std::string& myacct,
        std::string& hisnym,
        const std::string& amount,
        std::string& indices,
        bool hasPassword,
        std::shared_ptr<const Purse>& recipientCopy,
        std::shared_ptr<const Purse>& senderCopy) const;

#endif  // OT_CASH

private:
    friend opentxs::Factory;

    const api::Core& api_;
    const client::ServerAction& server_action_;

    Cash(const api::Core& api, const client::ServerAction& serverAction);
    Cash() = delete;
    Cash(const Cash&) = delete;
    Cash(Cash&&) = delete;
    Cash& operator=(const Cash&) = delete;
    Cash& operator=(Cash&&) = delete;
};
}  // namespace opentxs::api::client::implementation
