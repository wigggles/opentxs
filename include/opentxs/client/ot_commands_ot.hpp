#ifndef _H_OT_COMMANDS_OT
#define _H_OT_COMMANDS_OT

#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/util/Common.hpp>

#include <map>
#include <vector>

namespace opentxs
{

#define OT_COMMANDS_OT

using std::map;
using std::string;
using std::vector;

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
        const string& strMyAcctID, const string& strIndices,
        const string& strPaymentType);
    EXPORT OT_COMMANDS_OT static int32_t accept_inbox_items(
        const string& strMyAcctID, const int32_t nItemType,
        const string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_accept_invoices(
        const string& strMyAcctID, const string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_accept_payments(
        const string& strMyAcctID, const string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_account_balance(
        const string& strID);
    EXPORT OT_COMMANDS_OT static int32_t details_cancel_outgoing(
        const string& strMyNym, const string& strMyAcct,
        const string& strIndices);
    EXPORT OT_COMMANDS_OT static string details_check_user(
        const string& strServerID, const string& strMyNymID,
        const string& strHisNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_clear_expired(
        const string& strServerID, const string& strMyNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_clear_records(
        const string& strServerID, const string& strMyNymID,
        const string& strMyAcctID);
    EXPORT OT_COMMANDS_OT static int32_t details_confirm_plan(
        const string& strPlan, const int32_t nIndex);
    EXPORT OT_COMMANDS_OT static int32_t details_confirm_smart_contract(
        string& strSmartContract, const int32_t nIndex);
    EXPORT OT_COMMANDS_OT static int32_t details_create_nym(
        const int32_t nKeybits, const string& strName,
        const string& strSourceForNymID, const string& strAltLocation);
    EXPORT OT_COMMANDS_OT static int32_t details_create_offer(
        const string& strScale, const string& strMinIncrement,
        const string& strQuantity, const string& strPrice, const bool bSelling,
        const string& strLifespan);
    EXPORT OT_COMMANDS_OT static int32_t details_del_mail(
        const string& strMyNymID, const string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_del_outmail(
        const string& strMyNymID, const string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_deposit(
        const string& strServerID, const string& strMyAcctID);
    EXPORT OT_COMMANDS_OT static int32_t details_deposit_cheque(
        const string& strServerID, const string& strMyAcct,
        const string& strMyNymID, const string& strInstrument,
        const string& strType);
    EXPORT OT_COMMANDS_OT static int32_t details_deposit_purse(
        const string& strServerID, const string& strMyAcct,
        const string& strFromNymID, const string& strInstrument,
        const string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_discard_incoming(
        const string& strServer, const string& strMyNym,
        const string& strIndices);
    EXPORT OT_COMMANDS_OT static int32_t details_download_box_receipt(
        const string& strID, const int32_t nBoxType);
    EXPORT OT_COMMANDS_OT static int32_t details_download_contract(
        const string& strServerID, const string& strNymID,
        const string& strContractID);
    EXPORT OT_COMMANDS_OT static int32_t details_exchange_basket(
        const string& strServer, const string& strNym, const string& strAcct,
        const string& strBasketType);
    EXPORT OT_COMMANDS_OT static string details_export_cash(
        const string& strServerID, const string& strFromNymID,
        const string& strAssetTypeID, string& strHisNymID,
        const string& strIndices, const bool bPasswordProtected,
        string& strRetainedCopy);
    EXPORT OT_COMMANDS_OT static string details_export_nym(
        const string& strNymID);
    EXPORT OT_COMMANDS_OT static string details_get_nym_market_offers(
        const string& strServerID, const string& strNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_import_cash(
        const string& strInstrument);
    EXPORT OT_COMMANDS_OT static bool details_import_nym(
        const string& strNymImportFile, string& strOutNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_import_purse(
        const string& strInstrument, const bool bHasPassword,
        const string& strPurseOwner);
    EXPORT OT_COMMANDS_OT static int32_t details_kill_offer(
        const string& strServerID, const string& strNymID,
        const string& strAcctID, const string& strTransNum);
    EXPORT OT_COMMANDS_OT static int32_t details_new_basket(
        const string& strServer, const string& strNym);
    EXPORT OT_COMMANDS_OT static int32_t details_nym_stat(const string& strID);
    EXPORT OT_COMMANDS_OT static int32_t details_pay_dividend(
        const string& strAmount, const string& strMemo);
    EXPORT OT_COMMANDS_OT static int32_t details_propose_plan(
        const string& strServerID, const string& strMyNymID,
        const string& strMyAcctID, const string& strHisNymID,
        const string& strHisAcctID, const string& strDates,
        const string& strConsideration, const string& strInitialPayment,
        const string& strPaymentPlan, const string& strExpiry);
    EXPORT OT_COMMANDS_OT static bool details_refresh_nym(
        const string& strServerID, const string& strMyNymID,
        const bool bForceDownload);
    EXPORT OT_COMMANDS_OT static int32_t details_send_cash(
        string& strResponse, const string& strServerID,
        const string& strAssetTypeID, const string& strMyNymID,
        const string& strMyAcctID, string& strHisNymID, const string& strMemo,
        const string& strAmount, string& strIndices,
        const bool bPasswordProtected);
    EXPORT OT_COMMANDS_OT static int32_t details_send_transfer(
        const string& strMyAcctID, const string& strHisAcctID,
        const string& strAmount, const string& strNote);
    EXPORT OT_COMMANDS_OT static int32_t details_show_basket();
    EXPORT OT_COMMANDS_OT static bool details_show_credential(
        const string& strMyNymID, const string& strCredID);
    EXPORT OT_COMMANDS_OT static int32_t details_show_credentials(
        const string& strMyNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_show_expired(
        const string& strServerID, const string& strMyNymID,
        const int32_t nIndex, const string& strExpiredBox);
    EXPORT OT_COMMANDS_OT static int32_t details_show_expired_records(
        const string& strServerID, const string& strMyNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_show_market_offers(
        const string& strServerID, const string& strMarketID);
    EXPORT OT_COMMANDS_OT static int32_t details_show_nym_offers(
        const string& strServerID, const string& strNymID);
    EXPORT OT_COMMANDS_OT static int32_t details_show_record(
        const string& strServerID, const string& strMyNymID,
        const string& strMyAcctID, const int32_t nIndex,
        const string& strRecordBox);
    EXPORT OT_COMMANDS_OT static int32_t details_show_records(
        const string& strServerID, const string& strMyNymID,
        const string& strMyAcctID);
    EXPORT OT_COMMANDS_OT static int32_t details_stat_account(
        const string& strID);
    EXPORT OT_COMMANDS_OT static int32_t details_trigger_clause(
        const string& strServerID, const string& strMyNymID,
        const string& strTransNum, const string& strClause,
        const string& strParam);
    EXPORT OT_COMMANDS_OT static int32_t details_withdraw_cash(
        const string& strMyAcctID, const int64_t lAmount);
    EXPORT OT_COMMANDS_OT static int32_t details_withdraw_voucher(
        string& strOutput);
    EXPORT OT_COMMANDS_OT static int32_t details_write_cheque(
        string& strCheque, const bool bIsInvoice); // strCheque is output.
    EXPORT OT_COMMANDS_OT static int32_t download_acct_files();
    EXPORT OT_COMMANDS_OT static string find_masterID_for_subcred(
        const string& strMyNymID, const string& strInputID);
    EXPORT OT_COMMANDS_OT static string find_revokedID_for_subcred(
        const string& strMyNymID, const string& strInputID);
    EXPORT OT_COMMANDS_OT static int32_t handle_payment_index(
        const string& strMyAcctID, const int32_t nIndex,
        const string& strPaymentType,
        const string& strInbox); // (If nIndex is -1, then it will ask user to
                                 // paste an invoice.);
    EXPORT OT_COMMANDS_OT static int32_t impl_show_market_offers(
        string& strMarket);
    EXPORT OT_COMMANDS_OT static OTDB::MarketList* loadMarketList(
        const string& strerverID);
    EXPORT OT_COMMANDS_OT static OTDB::OfferListMarket* loadMarketOffers(
        const string& strerverID, const string& strMarketID);
    EXPORT OT_COMMANDS_OT static bool purse_get_indices_or_amount(
        const string& strServerID, const string& strAssetTypeID,
        const string& strMyNymID, int64_t& lAmount,
        string& strIndices); // If strIndices is input, lAmount is output. (And
                             // vice-versa.);
    EXPORT OT_COMMANDS_OT static bool show_mail_message(
        const string& strMyNymID, const int32_t nIndex,
        const bool bShowContents);
    EXPORT OT_COMMANDS_OT static bool show_outmail_message(
        const string& strMyNymID, const int32_t nIndex,
        const bool bShowContents);
    EXPORT OT_COMMANDS_OT static bool show_outpayment(const string& strMyNym,
                                                      const int32_t nIndex,
                                                      const bool bShowInFull);
    EXPORT OT_COMMANDS_OT static bool show_unconfirmed_parties(
        const string& strSmartContract, int32_t& nPartyCount);
    EXPORT OT_COMMANDS_OT static int32_t stat_basket_accounts(
        const string& strServer, const string& strNym, const bool bFilter,
        const string& strBasketType);
    EXPORT OT_COMMANDS_OT static bool stat_partyaccount(
        const string& strSmartContract, const string& strPartyName,
        const string& strAcctName, const int32_t nCurrentAccount);
    EXPORT OT_COMMANDS_OT static bool stat_partyaccount_index(
        const string& strSmartContract, const string& strPartyName,
        const int32_t nCurrentAccount);
    EXPORT OT_COMMANDS_OT static bool stat_partyaccounts(
        const string& strSmartContract, const string& strPartyName,
        const int32_t nDepth);
    EXPORT OT_COMMANDS_OT static bool stat_partyagent(
        const string& strSmartContract, const string& strPartyName,
        const string& strAgentName, const int32_t nIndex);
    EXPORT OT_COMMANDS_OT static bool stat_partyagent_index(
        const string& strSmartContract, const string& strPartyName,
        const int32_t nCurrentAgent);
    EXPORT OT_COMMANDS_OT static bool stat_partyagents(
        const string& strSmartContract, const string& strPartyName,
        const int32_t nDepth);
    EXPORT OT_COMMANDS_OT static bool withdraw_and_send_cash(
        const string& strMyAcctID, string& strHisNymID, const string& strMemo,
        const string& strAmount);

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

#endif
