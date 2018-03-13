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

#ifndef OPENTXS_API_CLIENT_CASH_IMPLEMENTATION_HPP
#define OPENTXS_API_CLIENT_CASH_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/api/client/Cash.hpp"
#include "opentxs/core/Lockable.hpp"

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
    bool withdraw_and_send_cash(
        const std::string& ACCT_ID,
        const std::string& RECIPIENT_NYM_ID,
        std::int64_t AMOUNT) const override;
#endif
private:
    friend api::implementation::Api;

    std::recursive_mutex& api_lock_;

    Cash(std::recursive_mutex& apilock);

    Cash() = delete;

    Cash(const Cash&) = delete;
    Cash(Cash&&) = delete;
    Cash& operator=(const Cash&) = delete;
    Cash& operator=(Cash&&) = delete;
};
}  // namespace opentxs::api::client::implementation

#endif  // OPENTXS_API_CLIENT_CASH_IMPLEMENTATION_HPP
