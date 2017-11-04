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

#ifndef OPENTXS_CORE_OTFOLDERS_HPP
#define OPENTXS_CORE_OTFOLDERS_HPP

#include "opentxs/api/Settings.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/String.hpp"

#include <string>

namespace opentxs
{

/** This class is for storing the names of the folders.  A instance of it must
 * be made. This function will store the folder-names automaticaly in the config
 * file. */
class OTFolders
{
private:
    static bool GetSetAll();

    static inline bool GetSetFolderName(
        api::Settings& config,
        std::string strKeyName,
        std::string strDefaultName,
        String& ret_strName)
    {
        if (ret_strName.Exists())
            return true;
        else {
            if (strKeyName.empty() || strDefaultName.empty()) return false;
            if (3 > strKeyName.size() || 3 > strDefaultName.size())
                return false;

            String strResult("");
            bool bIsNew(false);

            config.CheckSet_str(
                "folders",
                String(strKeyName),
                String(strDefaultName),
                strResult,
                bIsNew);

            if (!bIsNew)
                ret_strName = strResult;
            else
                ret_strName = strDefaultName.c_str();

            return true;
        }
    }

    static inline const String& GetFolder(const String& strFolder)
    {
        if (!strFolder.Exists()) {
            if (!GetSetAll()) {
                OT_FAIL;
            }
        }
        return strFolder;
    }

    static String s_strAccount;
    static String s_strCert;
    static String s_strCommon;
    static String s_strContract;
    static String s_strCron;
    static String s_strInbox;
    static String s_strMarket;
    static String s_strMint;
    static String s_strNym;
    static String s_strNymbox;
    static String s_strOutbox;
    static String s_strPaymentInbox;
    static String s_strPurse;
    static String s_strReceipt;
    static String s_strRecordBox;
    static String s_strExpiredBox;
    static String s_strScript;
    static String s_strSmartContracts;
    static String s_strSpent;
    static String s_strUserAcct;

public:
    EXPORT static const String& Account();
    EXPORT static const String& Cert();
    EXPORT static const String& Common();
    EXPORT static const String& Contract();
    EXPORT static const String& Cron();
    EXPORT static const String& Inbox();
    EXPORT static const String& Market();
    EXPORT static const String& Mint();
    EXPORT static const String& Nym();
    EXPORT static const String& Nymbox();
    EXPORT static const String& Outbox();
    EXPORT static const String& PaymentInbox();
    EXPORT static const String& Purse();
    EXPORT static const String& Receipt();
    EXPORT static const String& RecordBox();
    EXPORT static const String& ExpiredBox();
    EXPORT static const String& Script();
    EXPORT static const String& SmartContracts();
    EXPORT static const String& Spent();
    EXPORT static const String& UserAcct();
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_OTFOLDERS_HPP
