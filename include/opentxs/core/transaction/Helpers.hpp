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

#ifndef OPENTXS_CORE_TRANSACTION_HELPERS_HPP
#define OPENTXS_CORE_TRANSACTION_HELPERS_HPP

#include "opentxs/core/OTTransaction.hpp"

#include <cstdint>

namespace opentxs
{

class String;
class OTTransaction;
class Ledger;
class Identifier;
class NumList;

EXPORT const char* GetTransactionTypeString(int transactionNumber);

int32_t LoadAbbreviatedRecord(irr::io::IrrXMLReader*& xml,
                              int64_t& lNumberOfOrigin,
                              int64_t& lTransactionNum, int64_t& lInRefTo,
                              int64_t& lInRefDisplay, time64_t& the_DATE_SIGNED,
                              int& theType, String& strHash,
                              int64_t& lAdjustment, int64_t& lDisplayValue,
                              int64_t& lClosingNum, int64_t& lRequestNum,
                              bool& bReplyTransSuccess,
                              NumList* pNumList = nullptr);

EXPORT bool VerifyBoxReceiptExists(
    const Identifier& NOTARY_ID, const Identifier& NYM_ID,
    const Identifier& ACCOUNT_ID, // If for Nymbox (vs inbox/outbox) then
    // pass NYM_ID in this field also.
    int32_t nBoxType, // 0/nymbox, 1/inbox, 2/outbox
    const int64_t& lTransactionNum);

OTTransaction* LoadBoxReceipt(OTTransaction& theAbbrev, Ledger& theLedger);

EXPORT OTTransaction* LoadBoxReceipt(OTTransaction& theAbbrev,
                                     int64_t lLedgerType);

bool SetupBoxReceiptFilename(int64_t lLedgerType, OTTransaction& theTransaction,
                             const char* szCaller, String& strFolder1name,
                             String& strFolder2name, String& strFolder3name,
                             String& strFilename);

bool SetupBoxReceiptFilename(Ledger& theLedger, OTTransaction& theTransaction,
                             const char* szCaller, String& strFolder1name,
                             String& strFolder2name, String& strFolder3name,
                             String& strFilename);

bool SetupBoxReceiptFilename(int64_t lLedgerType, const String& strUserOrAcctID,
                             const String& strNotaryID,
                             const int64_t& lTransactionNum,
                             const char* szCaller, String& strFolder1name,
                             String& strFolder2name, String& strFolder3name,
                             String& strFilename);

} // namespace opentxs

#endif // OPENTXS_CORE_TRANSACTION_HELPERS_HPP
