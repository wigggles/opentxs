/************************************************************
*
*  ot_utility_ot.hpp
*
*/

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

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
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#ifndef __OT_UTILITY_OT_HPP__
#define __OT_UTILITY_OT_HPP__

#include "opentxs/core/util/Common.hpp"

#include <array>

namespace opentxs
{

#define OT_UTILITY_OT

inline OT_UTILITY_OT bool VerifyStringVal(const std::string& nValue)
{
    return 0 < nValue.length();
}

EXPORT OT_UTILITY_OT int32_t
InterpretTransactionMsgReply(const std::string& SERVER_ID,
                             const std::string& USER_ID,
                             const std::string& ACCOUNT_ID,
                             const std::string& strAttempt,
                             const std::string& strResponse);
EXPORT OT_UTILITY_OT bool VerifyExists(const std::string& name,
                                       const bool bFlag = true);
EXPORT OT_UTILITY_OT int32_t VerifyMessageSuccess(const std::string& message);
EXPORT OT_UTILITY_OT int32_t
VerifyMsgBalanceAgrmntSuccess(const std::string& SERVER_ID,
                              const std::string& USER_ID,
                              const std::string& ACCOUNT_ID,
                              const std::string& strMessage);
EXPORT OT_UTILITY_OT int32_t
VerifyMsgTrnxSuccess(const std::string& SERVER_ID, const std::string& USER_ID,
                     const std::string& ACCOUNT_ID,
                     const std::string& strMessage);

typedef std::array<bool, 4> OTfourbool;

class Utility
{
public:
    std::string strLastReplyReceived;
    int32_t delay_ms;
    int32_t max_trans_dl;

    EXPORT OT_UTILITY_OT Utility();
    EXPORT OT_UTILITY_OT ~Utility();

    EXPORT OT_UTILITY_OT void delay();
    EXPORT OT_UTILITY_OT int32_t
    getAndProcessNymbox_3(const std::string& serverID, const std::string& nymID,
                          bool& bWasMsgSent);
    EXPORT OT_UTILITY_OT int32_t
    getAndProcessNymbox_4(const std::string& serverID, const std::string& nymID,
                          bool& bWasMsgSent, const bool bForceDownload);
    EXPORT OT_UTILITY_OT int32_t
    getAndProcessNymbox_8(const std::string& serverID, const std::string& nymID,
                          bool& bWasMsgSent, const bool bForceDownload,
                          const int32_t nRequestNumber, bool& bFoundNymboxItem,
                          const bool bHarvestingForRetry,
                          const OTfourbool& bMsgFoursome);
    EXPORT OT_UTILITY_OT bool getBoxReceiptLowLevel(
        const std::string& serverID, const std::string& nymID,
        const std::string& accountID, const int32_t nBoxType,
        const int64_t strTransactionNum, bool& bWasSent);
    EXPORT OT_UTILITY_OT bool getBoxReceiptWithErrorCorrection(
        const std::string& serverID, const std::string& nymID,
        const std::string& accountID, const int32_t nBoxType,
        const int64_t strTransactionNum);
    EXPORT OT_UTILITY_OT int32_t
    getInboxAccount(const std::string& serverID, const std::string& nymID,
                    const std::string& accountID, bool& bWasSentInbox,
                    bool& bWasSentAccount);
    EXPORT OT_UTILITY_OT int32_t
    getInboxAccount(const std::string& serverID, const std::string& nymID,
                    const std::string& accountID, bool& bWasSentInbox,
                    bool& bWasSentAccount, const bool bForceDownload);
    EXPORT OT_UTILITY_OT int32_t
    getInboxAccount_old(const std::string& serverID, const std::string& nymID,
                        const std::string& accountID, bool& bWasSentInbox,
                        bool& bWasSentAccount, const bool bForceDownload);
    EXPORT OT_UTILITY_OT int32_t
    getInboxLowLevel(const std::string& serverID, const std::string& nymID,
                     const std::string& accountID, bool& bWasSent);
    EXPORT OT_UTILITY_OT int32_t
    getInboxLowLevel(const std::string& serverID, const std::string& nymID,
                     const std::string& accountID, bool& bWasSent,
                     const bool bForce);
    EXPORT OT_UTILITY_OT bool getInboxOutboxAccount(
        const std::string& accountID);
    EXPORT OT_UTILITY_OT bool getInboxOutboxAccount(
        const std::string& accountID, const bool bForceDownload);
    EXPORT OT_UTILITY_OT bool getIntermediaryFiles(
        const std::string& serverID, const std::string& nymID,
        const std::string& accountID);
    EXPORT OT_UTILITY_OT bool getIntermediaryFiles(const std::string& serverID,
                                                   const std::string& nymID,
                                                   const std::string& accountID,
                                                   const bool bForceDownload);
    EXPORT OT_UTILITY_OT bool getIntermediaryFiles_old(
        const std::string& serverID, const std::string& nymID,
        const std::string& accountID, const bool bForceDownload);
    EXPORT OT_UTILITY_OT std::string getLastReplyReceived();
    EXPORT OT_UTILITY_OT int32_t getNbrTransactionCount();
    EXPORT OT_UTILITY_OT int32_t
    getNymbox(const std::string& serverID, const std::string& nymID);
    EXPORT OT_UTILITY_OT int32_t getNymbox(const std::string& serverID,
                                           const std::string& nymID,
                                           const bool bForceDownload);
    EXPORT OT_UTILITY_OT int32_t
    getNymboxLowLevel(const std::string& serverID, const std::string& nymID);
    EXPORT OT_UTILITY_OT int32_t getNymboxLowLevel(const std::string& serverID,
                                                   const std::string& nymID,
                                                   bool& bWasSent);
    EXPORT OT_UTILITY_OT int32_t
    getOutboxLowLevel(const std::string& serverID, const std::string& nymID,
                      const std::string& accountID, bool& bWasSent);
    EXPORT OT_UTILITY_OT int32_t
    getOutboxLowLevel(const std::string& serverID, const std::string& nymID,
                      const std::string& accountID, bool& bWasSent,
                      const bool bForce);
    EXPORT OT_UTILITY_OT int32_t
    getRequestNumber(const std::string& serverID, const std::string& nymID);
    EXPORT OT_UTILITY_OT int32_t getRequestNumber(const std::string& serverID,
                                                  const std::string& nymID,
                                                  bool& bWasSent);
    EXPORT OT_UTILITY_OT bool getTransactionNumbers(const std::string& serverID,
                                                    const std::string& nymID);
    EXPORT OT_UTILITY_OT bool getTransactionNumbers(const std::string& serverID,
                                                    const std::string& nymID,
                                                    const bool bForceFirstCall);
    EXPORT OT_UTILITY_OT int32_t
    getTransactionNumLowLevel(const std::string& serverID,
                              const std::string& nymID, bool& bWasSent);
    EXPORT OT_UTILITY_OT bool insureHaveAllBoxReceipts(
        const std::string& serverID, const std::string& nymID,
        const std::string& accountID, const int32_t nBoxType);
    EXPORT OT_UTILITY_OT bool insureHaveAllBoxReceipts(
        const std::string& serverID, const std::string& nymID,
        const std::string& accountID, const int32_t nBoxType,
        const int32_t nRequestSeeking, bool& bFoundIt);
    EXPORT OT_UTILITY_OT void longDelay();
    EXPORT OT_UTILITY_OT int32_t
    processNymbox(const std::string& serverID, const std::string& nymID,
                  bool& bWasMsgSent, int32_t& nMsgSentRequestNumOut,
                  int32_t& nReplySuccessOut, int32_t& nBalanceSuccessOut,
                  int32_t& nTransSuccessOut);
    EXPORT OT_UTILITY_OT std::string ReceiveReplyLowLevel(
        const std::string& serverID17, const std::string& nymID,
        const int32_t nRequestNumber8, const std::string& IN_FUNCTION);
    EXPORT OT_UTILITY_OT int32_t
    receiveReplySuccessLowLevel(const std::string& serverID18,
                                const std::string& nymID,
                                const int32_t nRequestNumber7,
                                const std::string& IN_FUNCTION);
    EXPORT OT_UTILITY_OT int32_t
    sendProcessNymboxLowLevel(const std::string& serverID,
                              const std::string& nymID);
    EXPORT OT_UTILITY_OT void setLastReplyReceived(const std::string& strReply);
    EXPORT OT_UTILITY_OT void setNbrTransactionCount(int32_t new_trans_dl);
};

} // namespace opentxs

#endif // __OT_UTILITY_OT_HPP__
