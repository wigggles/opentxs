/************************************************************
 *
 *  ot_commands_ot.hpp
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

#ifndef OPENTXS_CLIENT_OT_COMMANDS_OT_HPP
#define OPENTXS_CLIENT_OT_COMMANDS_OT_HPP

#include "opentxs/core/util/Common.hpp"

#define OT_COMMANDS_OT

namespace opentxs
{

namespace OTDB
{
class MarketList;
class OfferListMarket;
}

class OT_Command
{
private:
    OT_Command()
    {
    }
    ~OT_Command()
    {
    }

public:
    EXPORT OT_COMMANDS_OT static int32_t accept_from_paymentbox(
        const std::string& strMyAcctID, const std::string& strIndices,
        const std::string& strPaymentType);
    EXPORT OT_COMMANDS_OT static int32_t accept_inbox_items(
        const std::string& strMyAcctID, int32_t nItemType,
        const std::string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_accept_invoices(
        const std::string& strMyAcctID, const std::string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_accept_payments(
        const std::string& strMyAcctID, const std::string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_account_balance(
        const std::string& strID);
    EXPORT OT_COMMANDS_OT static int32_t details_cancel_outgoing(
        const std::string& strMyNym, const std::string& strMyAcct,
        const std::string& strIndices);
    EXPORT OT_COMMANDS_OT static std::string details_check_user(
        const std::string& strServerID, const std::string& strMyNymID,
        const std::string& strHisNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_clear_expired(
        const std::string& strServerID, const std::string& strMyNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_clear_records(
        const std::string& strServerID, const std::string& strMyNymID,
        const std::string& strMyAcctID);
    EXPORT OT_COMMANDS_OT static int32_t details_confirm_plan(
        const std::string& strPlan, int32_t nIndex);
    EXPORT OT_COMMANDS_OT static int32_t details_confirm_smart_contract(
        std::string& strSmartContract, int32_t nIndex);
    EXPORT OT_COMMANDS_OT static int32_t details_create_nym(
        int32_t nKeybits, const std::string& strName,
        const std::string& strSourceForNymID,
        const std::string& strAltLocation);
    EXPORT OT_COMMANDS_OT static int32_t details_create_offer(
        const std::string& strScale, const std::string& strMinIncrement,
        const std::string& strQuantity, const std::string& strPrice,
        bool bSelling, const std::string& strLifespan);
    EXPORT OT_COMMANDS_OT static int32_t details_del_mail(
        const std::string& strMyNymID, const std::string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_del_outmail(
        const std::string& strMyNymID, const std::string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_deposit(
        const std::string& strServerID, const std::string& strMyAcctID);
    EXPORT OT_COMMANDS_OT static int32_t details_deposit_cheque(
        const std::string& strServerID, const std::string& strMyAcct,
        const std::string& strMyNymID, const std::string& strInstrument,
        const std::string& strType);
    EXPORT OT_COMMANDS_OT static int32_t details_deposit_purse(
        const std::string& strServerID, const std::string& strMyAcct,
        const std::string& strFromNymID, const std::string& strInstrument,
        const std::string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_discard_incoming(
        const std::string& strServer, const std::string& strMyNym,
        const std::string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_download_box_receipt(
        const std::string& strID, int32_t nBoxType);
    EXPORT OT_COMMANDS_OT static int32_t details_download_contract(
        const std::string& strServerID, const std::string& strNymID,
        const std::string& strContractID);
    EXPORT OT_COMMANDS_OT static int32_t details_exchange_basket(
        const std::string& strServer, const std::string& strNym,
        const std::string& strAcct, const std::string& strBasketType);
    EXPORT OT_COMMANDS_OT static std::string details_export_cash(
        const std::string& strServerID, const std::string& strFromNymID,
        const std::string& strAssetTypeID, std::string& strHisNymID,
        const std::string& strIndices, bool bPasswordProtected,
        std::string& strRetainedCopy);
    EXPORT OT_COMMANDS_OT static std::string details_export_nym(
        const std::string& strNymID);
    EXPORT OT_COMMANDS_OT static std::string details_get_nym_market_offers(
        const std::string& strServerID, const std::string& strNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_import_cash(
        const std::string& strInstrument);
    EXPORT OT_COMMANDS_OT static bool details_import_nym(
        const std::string& strNymImportFile, std::string& strOutNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_import_purse(
        const std::string& strInstrument, bool bHasPassword,
        const std::string& strPurseOwner);
    EXPORT OT_COMMANDS_OT static int32_t details_kill_offer(
        const std::string& strServerID, const std::string& strNymID,
        const std::string& strAcctID, const std::string& strTransNum);
    EXPORT OT_COMMANDS_OT static int32_t details_new_basket(
        const std::string& strServer, const std::string& strNym);
    EXPORT OT_COMMANDS_OT static int32_t details_nym_stat(
        const std::string& strID);
    EXPORT OT_COMMANDS_OT static int32_t details_pay_dividend(
        const std::string& strAmount, const std::string& strMemo);
    EXPORT OT_COMMANDS_OT static int32_t details_propose_plan(
        const std::string& strServerID, const std::string& strMyNymID,
        const std::string& strMyAcctID, const std::string& strHisNymID,
        const std::string& strHisAcctID, const std::string& strDates,
        const std::string& strConsideration,
        const std::string& strInitialPayment, const std::string& strPaymentPlan,
        const std::string& strExpiry);
    EXPORT OT_COMMANDS_OT static bool details_refresh_nym(
        const std::string& strServerID, const std::string& strMyNymID,
        bool bForceDownload);
    EXPORT OT_COMMANDS_OT static int32_t details_send_cash(
        std::string& strResponse, const std::string& strServerID,
        const std::string& strAssetTypeID, const std::string& strMyNymID,
        const std::string& strMyAcctID, std::string& strHisNymID,
        const std::string& strMemo, const std::string& strAmount,
        std::string& strIndices, bool bPasswordProtected);
    EXPORT OT_COMMANDS_OT static int32_t details_send_transfer(
        const std::string& strMyAcctID, const std::string& strHisAcctID,
        const std::string& strAmount, const std::string& strNote);
    EXPORT OT_COMMANDS_OT static int32_t details_show_basket();
    EXPORT OT_COMMANDS_OT static bool details_show_credential(
        const std::string& strMyNymID, const std::string& strCredID);
    EXPORT OT_COMMANDS_OT static int32_t details_show_credentials(
        const std::string& strMyNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_show_expired(
        const std::string& strServerID, const std::string& strMyNymID,
        int32_t nIndex, const std::string& strExpiredBox);
    EXPORT OT_COMMANDS_OT static int32_t details_show_expired_records(
        const std::string& strServerID, const std::string& strMyNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_show_market_offers(
        const std::string& strServerID, const std::string& strMarketID);
    EXPORT OT_COMMANDS_OT static int32_t details_show_nym_offers(
        const std::string& strServerID, const std::string& strNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_show_record(
        const std::string& strServerID, const std::string& strMyNymID,
        const std::string& strMyAcctID, int32_t nIndex,
        const std::string& strRecordBox);
    EXPORT OT_COMMANDS_OT static int32_t details_show_records(
        const std::string& strServerID, const std::string& strMyNymID,
        const std::string& strMyAcctID);
    EXPORT OT_COMMANDS_OT static int32_t details_stat_account(
        const std::string& strID);
    EXPORT OT_COMMANDS_OT static int32_t details_trigger_clause(
        const std::string& strServerID, const std::string& strMyNymID,
        const std::string& strTransNum, const std::string& strClause,
        const std::string& strParam);
    EXPORT OT_COMMANDS_OT static int32_t details_withdraw_cash(
        const std::string& strMyAcctID, int64_t lAmount);
    EXPORT OT_COMMANDS_OT static int32_t details_withdraw_voucher(
        std::string& strOutput);
    EXPORT OT_COMMANDS_OT static int32_t details_write_cheque(
        std::string& strCheque, bool bIsInvoice); // strCheque is output.
    EXPORT OT_COMMANDS_OT static int32_t download_acct_files();
    EXPORT OT_COMMANDS_OT static std::string find_masterID_for_subcred(
        const std::string& strMyNymID, const std::string& strInputID);
    EXPORT OT_COMMANDS_OT static std::string find_revokedID_for_subcred(
        const std::string& strMyNymID, const std::string& strInputID);
    EXPORT OT_COMMANDS_OT static int32_t handle_payment_index(
        const std::string& strMyAcctID, int32_t nIndex,
        const std::string& strPaymentType,
        const std::string& strInbox); // (If nIndex is -1, then it will ask user
                                      // to
                                      // paste an invoice.);
    EXPORT OT_COMMANDS_OT static int32_t impl_show_market_offers(
        std::string& strMarket);
    EXPORT OT_COMMANDS_OT static OTDB::MarketList* loadMarketList(
        const std::string& strerverID);
    EXPORT OT_COMMANDS_OT static OTDB::OfferListMarket* loadMarketOffers(
        const std::string& strerverID, const std::string& strMarketID);
    EXPORT OT_COMMANDS_OT static bool purse_get_indices_or_amount(
        const std::string& strServerID, const std::string& strAssetTypeID,
        const std::string& strMyNymID, int64_t& lAmount,
        std::string& strIndices); // If strIndices is input, lAmount is output.
                                  // (And
                                  // vice-versa.);
    EXPORT OT_COMMANDS_OT static bool show_mail_message(
        const std::string& strMyNymID, int32_t nIndex, bool bShowContents);
    EXPORT OT_COMMANDS_OT static bool show_outmail_message(
        const std::string& strMyNymID, int32_t nIndex, bool bShowContents);
    EXPORT OT_COMMANDS_OT static bool show_outpayment(
        const std::string& strMyNym, int32_t nIndex, bool bShowInFull);
    EXPORT OT_COMMANDS_OT static bool show_unconfirmed_parties(
        const std::string& strSmartContract, int32_t& nPartyCount);
    EXPORT OT_COMMANDS_OT static int32_t stat_basket_accounts(
        const std::string& strServer, const std::string& strNym, bool bFilter,
        const std::string& strBasketType);
    EXPORT OT_COMMANDS_OT static bool stat_partyaccount(
        const std::string& strSmartContract, const std::string& strPartyName,
        const std::string& strAcctName, int32_t nCurrentAccount);
    EXPORT OT_COMMANDS_OT static bool stat_partyaccount_index(
        const std::string& strSmartContract, const std::string& strPartyName,
        int32_t nCurrentAccount);
    EXPORT OT_COMMANDS_OT static bool stat_partyaccounts(
        const std::string& strSmartContract, const std::string& strPartyName,
        int32_t nDepth);
    EXPORT OT_COMMANDS_OT static bool stat_partyagent(
        const std::string& strSmartContract, const std::string& strPartyName,
        const std::string& strAgentName, int32_t nIndex);
    EXPORT OT_COMMANDS_OT static bool stat_partyagent_index(
        const std::string& strSmartContract, const std::string& strPartyName,
        int32_t nCurrentAgent);
    EXPORT OT_COMMANDS_OT static bool stat_partyagents(
        const std::string& strSmartContract, const std::string& strPartyName,
        int32_t nDepth);
    EXPORT OT_COMMANDS_OT static bool withdraw_and_send_cash(
        const std::string& strMyAcctID, std::string& strHisNymID,
        const std::string& strMemo, const std::string& strAmount);

    // opentxs commands
    EXPORT OT_COMMANDS_OT static int32_t mainAcceptAll();
    EXPORT OT_COMMANDS_OT static int32_t mainAcceptInbox();
    EXPORT OT_COMMANDS_OT static int32_t mainAcceptInvoices();
    EXPORT OT_COMMANDS_OT static int32_t mainAcceptMoney();
    EXPORT OT_COMMANDS_OT static int32_t mainAcceptPayments();
    EXPORT OT_COMMANDS_OT static int32_t mainAcceptReceipts();
    EXPORT OT_COMMANDS_OT static int32_t mainAcceptTransfers();
    EXPORT OT_COMMANDS_OT static int32_t mainAddAsset();
    EXPORT OT_COMMANDS_OT static int32_t mainAddServer();
    EXPORT OT_COMMANDS_OT static int32_t mainAddSignature();
    EXPORT OT_COMMANDS_OT static int32_t mainAdjustUsageCredits();
    EXPORT OT_COMMANDS_OT static int32_t mainCancel();
    EXPORT OT_COMMANDS_OT static int32_t mainChangePw();
    EXPORT OT_COMMANDS_OT static int32_t mainCheckNym();
    EXPORT OT_COMMANDS_OT static int32_t mainClearExpired();
    EXPORT OT_COMMANDS_OT static int32_t mainClearRecords();
    EXPORT OT_COMMANDS_OT static int32_t mainConfirm();
    EXPORT OT_COMMANDS_OT static int32_t mainDecode();
    EXPORT OT_COMMANDS_OT static int32_t mainDecrypt();
    EXPORT OT_COMMANDS_OT static int32_t mainDeleteInmail();
    EXPORT OT_COMMANDS_OT static int32_t mainDeleteOutmail();
    EXPORT OT_COMMANDS_OT static int32_t mainDeposit();
    EXPORT OT_COMMANDS_OT static int32_t mainDiscard();
    EXPORT OT_COMMANDS_OT static int32_t mainEditAccount();
    EXPORT OT_COMMANDS_OT static int32_t mainEditAsset();
    EXPORT OT_COMMANDS_OT static int32_t mainEditNym();
    EXPORT OT_COMMANDS_OT static int32_t mainEditServer();
    EXPORT OT_COMMANDS_OT static int32_t mainEncode();
    EXPORT OT_COMMANDS_OT static int32_t mainEncrypt();
    EXPORT OT_COMMANDS_OT static int32_t mainExchangeBasket();
    EXPORT OT_COMMANDS_OT static int32_t mainExportCash();
    EXPORT OT_COMMANDS_OT static int32_t mainExportNym();
    EXPORT OT_COMMANDS_OT static int32_t mainGetContract();
    EXPORT OT_COMMANDS_OT static int32_t mainGetMarkets();
    EXPORT OT_COMMANDS_OT static int32_t mainGetMyOffers();
    EXPORT OT_COMMANDS_OT static int32_t mainGetOffers();
    EXPORT OT_COMMANDS_OT static int32_t mainGetReceipt();
    EXPORT OT_COMMANDS_OT static int32_t mainImportCash();
    EXPORT OT_COMMANDS_OT static int32_t mainImportNym();
    EXPORT OT_COMMANDS_OT static int32_t mainInbox();
    EXPORT OT_COMMANDS_OT static int32_t mainInmail();
    EXPORT OT_COMMANDS_OT static int32_t mainInpayments();
    EXPORT OT_COMMANDS_OT static int32_t mainIssueAsset();
    EXPORT OT_COMMANDS_OT static int32_t mainKillOffer();
    EXPORT OT_COMMANDS_OT static int32_t mainKillPlan();
    EXPORT OT_COMMANDS_OT static int32_t mainNewAccount();
    EXPORT OT_COMMANDS_OT static int32_t mainNewAsset();
    EXPORT OT_COMMANDS_OT static int32_t mainNewBasket();
    EXPORT OT_COMMANDS_OT static int32_t mainNewCredential();
    EXPORT OT_COMMANDS_OT static int32_t mainNewKey();
    EXPORT OT_COMMANDS_OT static int32_t mainNewNym();
    EXPORT OT_COMMANDS_OT static int32_t mainNewOffer();
    EXPORT OT_COMMANDS_OT static int32_t mainNewServer();
    EXPORT OT_COMMANDS_OT static int32_t mainOutbox();
    EXPORT OT_COMMANDS_OT static int32_t mainOutmail();
    EXPORT OT_COMMANDS_OT static int32_t mainOutpayment();
    EXPORT OT_COMMANDS_OT static int32_t mainPasswordDecrypt();
    EXPORT OT_COMMANDS_OT static int32_t mainPasswordEncrypt();
    EXPORT OT_COMMANDS_OT static int32_t mainPayDividend();
    EXPORT OT_COMMANDS_OT static int32_t mainPayInvoice();
    EXPORT OT_COMMANDS_OT static int32_t mainProposePlan();
    EXPORT OT_COMMANDS_OT static int32_t mainRefresh();
    EXPORT OT_COMMANDS_OT static int32_t mainRefreshAccount();
    EXPORT OT_COMMANDS_OT static int32_t mainRefreshNym();
    EXPORT OT_COMMANDS_OT static int32_t mainRegisterNym();
    EXPORT OT_COMMANDS_OT static int32_t mainRevokeCredential();
    EXPORT OT_COMMANDS_OT static int32_t mainSendCash();
    EXPORT OT_COMMANDS_OT static int32_t mainSendCheque();
    EXPORT OT_COMMANDS_OT static int32_t mainSendInvoice();
    EXPORT OT_COMMANDS_OT static int32_t mainSendMessage();
    EXPORT OT_COMMANDS_OT static int32_t mainSendVoucher();
    EXPORT OT_COMMANDS_OT static int32_t mainShowAccount();
    EXPORT OT_COMMANDS_OT static int32_t mainShowAccounts();
    EXPORT OT_COMMANDS_OT static int32_t mainShowActive();
    EXPORT OT_COMMANDS_OT static int32_t mainShowAssets();
    EXPORT OT_COMMANDS_OT static int32_t mainShowBalance();
    EXPORT OT_COMMANDS_OT static int32_t mainShowBasket();
    EXPORT OT_COMMANDS_OT static int32_t mainShowCredential();
    EXPORT OT_COMMANDS_OT static int32_t mainShowCredentials();
    EXPORT OT_COMMANDS_OT static int32_t mainShowExpired();
    EXPORT OT_COMMANDS_OT static int32_t mainShowIncoming();
    EXPORT OT_COMMANDS_OT static int32_t mainShowMarkets();
    EXPORT OT_COMMANDS_OT static int32_t mainShowMint();
    EXPORT OT_COMMANDS_OT static int32_t mainShowMyOffers();
    EXPORT OT_COMMANDS_OT static int32_t mainShowNym();
    EXPORT OT_COMMANDS_OT static int32_t mainShowNyms();
    EXPORT OT_COMMANDS_OT static int32_t mainShowOffers();
    EXPORT OT_COMMANDS_OT static int32_t mainShowOutgoing();
    EXPORT OT_COMMANDS_OT static int32_t mainShowPayment();
    EXPORT OT_COMMANDS_OT static int32_t mainShowPurse();
    EXPORT OT_COMMANDS_OT static int32_t mainShowRecords();
    EXPORT OT_COMMANDS_OT static int32_t mainShowServers();
    EXPORT OT_COMMANDS_OT static int32_t mainShowWallet();
    EXPORT OT_COMMANDS_OT static int32_t mainSignContract();
    EXPORT OT_COMMANDS_OT static int32_t mainTransfer();
    EXPORT OT_COMMANDS_OT static int32_t mainTriggerClause();
    EXPORT OT_COMMANDS_OT static int32_t mainVerifyReceipt();
    EXPORT OT_COMMANDS_OT static int32_t mainVerifySignature();
    EXPORT OT_COMMANDS_OT static int32_t mainWithdrawCash();
    EXPORT OT_COMMANDS_OT static int32_t mainWithdrawVoucher();
    EXPORT OT_COMMANDS_OT static int32_t mainWriteCheque();
    EXPORT OT_COMMANDS_OT static int32_t mainWriteInvoice();
};

} // namespace opentxs

#endif // OPENTXS_CLIENT_OT_COMMANDS_OT_HPP
