// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_UTIL_OTFOLDERS_HPP
#define OPENTXS_CORE_UTIL_OTFOLDERS_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/api/Settings.hpp"
#include "opentxs/core/Log.hpp"
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

            auto strResult = String::Factory();
            bool bIsNew(false);

            config.CheckSet_str(
                String::Factory("folders"),
                String::Factory(strKeyName),
                String::Factory(strDefaultName),
                strResult,
                bIsNew);

            if (!bIsNew)
                ret_strName.Set(strResult->Get());
            else
                ret_strName.Set(strDefaultName.c_str());

            return true;
        }
    }

    static inline const String& GetFolder(const String& strFolder)
    {
        if (!strFolder.Exists()) {
            if (!GetSetAll()) { OT_FAIL; }
        }
        return strFolder;
    }

    static OTString s_strAccount;
    static OTString s_strCert;
    static OTString s_strCommon;
    static OTString s_strContract;
    static OTString s_strCron;
    static OTString s_strInbox;
    static OTString s_strMarket;
    static OTString s_strMint;
    static OTString s_strNym;
    static OTString s_strNymbox;
    static OTString s_strOutbox;
    static OTString s_strPaymentInbox;
    static OTString s_strPurse;
    static OTString s_strReceipt;
    static OTString s_strRecordBox;
    static OTString s_strExpiredBox;
    static OTString s_strSpent;
    static OTString s_strUserAcct;

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
    EXPORT static const String& Spent();
    EXPORT static const String& UserAcct();
};
}  // namespace opentxs
#endif
