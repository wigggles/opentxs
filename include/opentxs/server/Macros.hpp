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

#ifndef OPENTXS_SERVER_MACROS_HPP
#define OPENTXS_SERVER_MACROS_HPP

#include "ServerSettings.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs
{

// For NYM_IS_ALLOWED() to evaluate to TRUE, either the boolean value itself is
// set to true (meaning, "YES any Nym is allowed..") OR (it only continues if
// that part fails) if the override Nym's ID matches to the Nym ID passed in (as
// a const char *).
#define NYM_IS_ALLOWED(SZ_NYM_ID, BOOL_VAR_NAME)                               \
    ((BOOL_VAR_NAME) ||                                                        \
     ((ServerSettings::GetOverrideNymID().size() > 0) &&                       \
      (0 == ServerSettings::GetOverrideNymID().compare((SZ_NYM_ID)))))

#define OT_ENFORCE_PERMISSION_MSG(BOOL_VAR_NAME)                               \
    {                                                                          \
        const char* pNymAllowedIDStr = theMessage.m_strNymID.Get();            \
        const char* pActionNameStr = theMessage.m_strCommand.Get();            \
                                                                               \
        if (false == NYM_IS_ALLOWED(pNymAllowedIDStr, BOOL_VAR_NAME)) {        \
            Log::vOutput(0, "Nym %s attempted an action (%s), but based on "   \
                            "server configuration, he's not allowed.\n",       \
                         pNymAllowedIDStr, pActionNameStr);                    \
            return false;                                                      \
        }                                                                      \
    }

} // namespace opentxs

#endif // OPENTXS_SERVER_MACROS_HPP
