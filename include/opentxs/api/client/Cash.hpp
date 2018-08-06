// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_CASH_HPP
#define OPENTXS_API_CLIENT_CASH_HPP

#include "opentxs/Forward.hpp"

#include <cstdint>
#include <string>

namespace opentxs
{
namespace api
{
namespace client
{
class Cash
{
public:
#if OT_CASH
    EXPORT virtual bool deposit_cash(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCT_ID,
        const std::string& STR_PURSE) const = 0;
    EXPORT virtual bool deposit_local_purse(
        const std::string& NOTARY_ID,
        const std::string& NYM_ID,
        const std::string& ACCT_ID,
        const std::string& STR_INDICES) const = 0;
    EXPORT virtual std::int32_t deposit_purse(
        const std::string& server,
        const std::string& myacct,
        const std::string& mynym,
        std::string instrument,
        const std::string& indices,
        std::string* pOptionalOutput = nullptr) const = 0;
    EXPORT virtual bool easy_withdraw_cash(
        const std::string& ACCT_ID,
        std::int64_t AMOUNT) const = 0;
    EXPORT virtual std::string export_cash(
        const std::string& NOTARY_ID,
        const std::string& FROM_NYM_ID,
        const std::string& INSTRUMENT_DEFINITION_ID,
        const std::string& TO_NYM_ID,
        const std::string& STR_INDICES,
        bool bPasswordProtected,
        std::string& STR_RETAINED_COPY) const = 0;
    EXPORT virtual std::int32_t send_cash(
        std::string& response,
        const std::string& server,
        const std::string& mynym,
        const std::string& assetType,
        const std::string& myacct,
        std::string& hisnym,
        const std::string& amount,
        std::string& indices,
        bool hasPassword) const = 0;
    EXPORT virtual bool withdraw_and_export_cash(
        const std::string& ACCT_ID,
        const std::string& RECIPIENT_NYM_ID,
        std::int64_t AMOUNT,
        std::shared_ptr<const Purse>& recipientCopy,
        std::shared_ptr<const Purse>& senderCopy,
        bool bPasswordProtected = false) const = 0;
    EXPORT virtual bool withdraw_and_send_cash(
        const std::string& ACCT_ID,
        const std::string& RECIPIENT_NYM_ID,
        std::int64_t AMOUNT) const = 0;

#endif
    EXPORT virtual ~Cash() = default;

protected:
    Cash() = default;

private:
    Cash(const Cash&) = delete;
    Cash(Cash&&) = delete;
    Cash& operator=(const Cash&) = delete;
    Cash& operator=(Cash&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs

#endif
