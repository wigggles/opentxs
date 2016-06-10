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

#include "opentxs/server/ConfigLoader.hpp"
#include "opentxs/server/ServerSettings.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/util/OTDataFolder.hpp"
#include "opentxs/core/app/App.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTKeyring.hpp"
#include <cstdint>

#define SERVER_WALLET_FILENAME "notaryServer.xml"
#define SERVER_MASTER_KEY_TIMEOUT_DEFAULT -1
#define SERVER_USE_SYSTEM_KEYRING false

namespace opentxs
{

bool ConfigLoader::load(String& walletFilename)
{
    const char* szFunc = "ConfigLoader::load()";

    // Setup Config File
    String strConfigFolder, strConfigFilename;

    if (!OTDataFolder::IsInitialized()) {
        OT_FAIL;
    }

    // LOG LEVEL
    {
        bool bIsNewKey;
        int64_t lValue;
        App::Me().Config().CheckSet_long("logging", "log_level", 0, lValue, bIsNewKey);
        Log::SetLogLevel(static_cast<int32_t>(lValue));
    }

    // WALLET

    // WALLET FILENAME
    //
    // Clean and Set
    {
        bool bIsNewKey;
        String strValue;
        App::Me().Config().CheckSet_str("wallet", "wallet_filename",
                               SERVER_WALLET_FILENAME, strValue, bIsNewKey);
        walletFilename.Set(strValue);
        Log::vOutput(0, "Using Wallet: %s\n", strValue.Get());
    }

    // CRON
    {
        const char* szComment = ";; CRON  (regular events like market trades "
                                "and smart contract clauses)\n";

        bool b_SectionExist;
        App::Me().Config().CheckSetSection("cron", szComment, b_SectionExist);
    }

    {
        const char* szComment = "; refill_trans_number is the count of "
                                "transaction numbers cron will grab for "
                                "itself,\n"
                                "; whenever its supply is getting low.  If it "
                                "ever drops below 20% of this count\n"
                                "; while in the middle of processing, it will "
                                "put a WARNING into your server log.\n";

        bool bIsNewKey;
        int64_t lValue;
        App::Me().Config().CheckSet_long("cron", "refill_trans_number", 500, lValue,
                                bIsNewKey, szComment);
        OTCron::SetCronRefillAmount(static_cast<int32_t>(lValue));
    }

    {
        const char* szComment = "; ms_between_cron_beats is the number of "
                                "milliseconds before Cron processes\n"
                                "; (all the trades, all the smart contracts, "
                                "etc every 10 seconds.)\n";

        bool bIsNewKey;
        int64_t lValue;
        App::Me().Config().CheckSet_long("cron", "ms_between_cron_beats", 10000, lValue,
                                bIsNewKey, szComment);
        OTCron::SetCronMsBetweenProcess(static_cast<int32_t>(lValue));
    }

    {
        const char* szComment = "; max_items_per_nym is the number of cron "
                                "items (such as market offers or payment\n"
                                "; plans) that any given Nym is allowed to "
                                "have live and active at the same time.\n";

        bool bIsNewKey;
        int64_t lValue;
        App::Me().Config().CheckSet_long("cron", "max_items_per_nym", 10, lValue,
                                bIsNewKey, szComment);
        OTCron::SetCronMaxItemsPerNym(static_cast<int32_t>(lValue));
    }

    // HEARTBEAT

    {
        const char* szComment = ";; HEARTBEAT\n";

        bool bSectionExist;
        App::Me().Config().CheckSetSection("heartbeat", szComment, bSectionExist);
    }

    {
        const char* szComment = "; no_requests is the number of client "
                                "requests the server processes per "
                                "heartbeat.\n";

        bool bIsNewKey;
        int64_t lValue;
        App::Me().Config().CheckSet_long("heartbeat", "no_requests", 10, lValue,
                                bIsNewKey, szComment);
        ServerSettings::SetHeartbeatNoRequests(static_cast<int32_t>(lValue));
    }

    {
        const char* szComment = "; ms_between_beats is the number of "
                                "milliseconds between each heartbeat.\n";

        bool bIsNewKey;
        int64_t lValue;
        App::Me().Config().CheckSet_long("heartbeat", "ms_between_beats", 100, lValue,
                                bIsNewKey, szComment);
        ServerSettings::SetHeartbeatMsBetweenBeats(
            static_cast<int32_t>(lValue));
    }

    // PERMISSIONS

    {
        const char* szComment = ";; PERMISSIONS\n"
                                ";; You can deactivate server functions here "
                                "by setting them to false.\n"
                                ";; (Even if you do, override_nym_id will "
                                "STILL be able to do those functions.)\n";

        bool bSectionExists;
        App::Me().Config().CheckSetSection("permissions", szComment, bSectionExists);
    }

    {
        String strValue;
        const char* szValue;

        std::string stdstrValue = ServerSettings::GetOverrideNymID();
        szValue = stdstrValue.c_str();

        bool bIsNewKey;

        if (nullptr == szValue)
            App::Me().Config().CheckSet_str("permissions", "override_nym_id", nullptr,
                                   strValue, bIsNewKey);
        else
            App::Me().Config().CheckSet_str("permissions", "override_nym_id", szValue,
                                   strValue, bIsNewKey);

        ServerSettings::SetOverrideNymID(strValue.Get());
    }

    // MARKETS

    {
        const char* szComment = "; minimum_scale is the smallest allowed "
                                "power-of-ten for the scale, for any market.\n"
                                "; (1oz, 10oz, 100oz, 1000oz.)\n";

        bool bIsNewKey;
        int64_t lValue;
        App::Me().Config().CheckSet_long("markets", "minimum_scale",
                                ServerSettings::GetMinMarketScale(), lValue,
                                bIsNewKey, szComment);
        ServerSettings::SetMinMarketScale(lValue);
    }

    // SECURITY (beginnings of..)

    // Master Key Timeout
    {
        const char* szComment =
            "; master_key_timeout is how int64_t the master key will be in "
            "memory until a thread wipes it out.\n"
            "; 0   : means you have to type your password EVERY time OT uses a "
            "private key. (Even multiple times in a single function.)\n"
            "; 300 : means you only have to type it once per 5 minutes.\n"
            "; -1  : means you only type it once PER RUN (popular for "
            "servers.)\n";

        bool bIsNewKey;
        int64_t lValue;
        App::Me().Config().CheckSet_long("security", "master_key_timeout",
                                SERVER_MASTER_KEY_TIMEOUT_DEFAULT, lValue,
                                bIsNewKey, szComment);
        OTCachedKey::It()->SetTimeoutSeconds(static_cast<int32_t>(lValue));
    }

    // Use System Keyring
    {
        bool bIsNewKey;
        bool bValue;
        App::Me().Config().CheckSet_bool("security", "use_system_keyring",
                                SERVER_USE_SYSTEM_KEYRING, bValue, bIsNewKey);
        OTCachedKey::It()->UseSystemKeyring(bValue);

#if defined(OT_KEYRING_FLATFILE)
        // Is there a password folder? (There shouldn't be, but we allow it...)
        //
        if (bValue) {
            bool bIsNewKey2;
            String strValue;
            App::Me().Config().CheckSet_str("security", "password_folder", "", strValue,
                                   bIsNewKey2);
            if (strValue.Exists()) {
                OTKeyring::FlatFile_SetPasswordFolder(strValue.Get());
                Log::vOutput(0, " Using server password folder: %s\n",
                             strValue.Get());
            }
        }
#endif
    }

    // (#defined right above this function.)
    //

    App::Me().Config().SetOption_bool("permissions", "admin_usage_credits",
                             ServerSettings::__admin_usage_credits);
    App::Me().Config().SetOption_bool("permissions", "admin_server_locked",
                             ServerSettings::__admin_server_locked);
    App::Me().Config().SetOption_bool("permissions", "cmd_usage_credits",
                             ServerSettings::__cmd_usage_credits);
    App::Me().Config().SetOption_bool("permissions", "cmd_issue_asset",
                             ServerSettings::__cmd_issue_asset);
    App::Me().Config().SetOption_bool("permissions", "cmd_get_contract",
                             ServerSettings::__cmd_get_contract);
    App::Me().Config().SetOption_bool("permissions", "cmd_check_notary_id",
                             ServerSettings::__cmd_check_notary_id);
    App::Me().Config().SetOption_bool("permissions", "cmd_create_user_acct",
                             ServerSettings::__cmd_create_user_acct);
    App::Me().Config().SetOption_bool("permissions", "cmd_del_user_acct",
                             ServerSettings::__cmd_del_user_acct);
    App::Me().Config().SetOption_bool("permissions", "cmd_check_nym",
                             ServerSettings::__cmd_check_nym);
    App::Me().Config().SetOption_bool("permissions", "cmd_get_requestnumber",
                             ServerSettings::__cmd_get_requestnumber);
    App::Me().Config().SetOption_bool("permissions", "cmd_get_trans_nums",
                             ServerSettings::__cmd_get_trans_nums);
    App::Me().Config().SetOption_bool("permissions", "cmd_send_message",
                             ServerSettings::__cmd_send_message);
    App::Me().Config().SetOption_bool("permissions", "cmd_get_nymbox",
                             ServerSettings::__cmd_get_nymbox);
    App::Me().Config().SetOption_bool("permissions", "cmd_process_nymbox",
                             ServerSettings::__cmd_process_nymbox);
    App::Me().Config().SetOption_bool("permissions", "cmd_create_asset_acct",
                             ServerSettings::__cmd_create_asset_acct);
    App::Me().Config().SetOption_bool("permissions", "cmd_del_asset_acct",
                             ServerSettings::__cmd_del_asset_acct);
    App::Me().Config().SetOption_bool("permissions", "cmd_get_acct",
                             ServerSettings::__cmd_get_acct);
    App::Me().Config().SetOption_bool("permissions", "cmd_get_inbox",
                             ServerSettings::__cmd_get_inbox);
    App::Me().Config().SetOption_bool("permissions", "cmd_get_outbox",
                             ServerSettings::__cmd_get_outbox);
    App::Me().Config().SetOption_bool("permissions", "cmd_process_inbox",
                             ServerSettings::__cmd_process_inbox);
    App::Me().Config().SetOption_bool("permissions", "cmd_issue_basket",
                             ServerSettings::__cmd_issue_basket);
    App::Me().Config().SetOption_bool("permissions", "transact_exchange_basket",
                             ServerSettings::__transact_exchange_basket);
    App::Me().Config().SetOption_bool("permissions", "cmd_notarize_transaction",
                             ServerSettings::__cmd_notarize_transaction);
    App::Me().Config().SetOption_bool("permissions", "transact_process_inbox",
                             ServerSettings::__transact_process_inbox);
    App::Me().Config().SetOption_bool("permissions", "transact_transfer",
                             ServerSettings::__transact_transfer);
    App::Me().Config().SetOption_bool("permissions", "transact_withdrawal",
                             ServerSettings::__transact_withdrawal);
    App::Me().Config().SetOption_bool("permissions", "transact_deposit",
                             ServerSettings::__transact_deposit);
    App::Me().Config().SetOption_bool("permissions", "transact_withdraw_voucher",
                             ServerSettings::__transact_withdraw_voucher);
    App::Me().Config().SetOption_bool("permissions", "transact_pay_dividend",
                             ServerSettings::__transact_pay_dividend);
    App::Me().Config().SetOption_bool("permissions", "transact_deposit_cheque",
                             ServerSettings::__transact_deposit_cheque);
    App::Me().Config().SetOption_bool("permissions", "cmd_get_mint",
                             ServerSettings::__cmd_get_mint);
    App::Me().Config().SetOption_bool("permissions", "transact_withdraw_cash",
                             ServerSettings::__transact_withdraw_cash);
    App::Me().Config().SetOption_bool("permissions", "transact_deposit_cash",
                             ServerSettings::__transact_deposit_cash);
    App::Me().Config().SetOption_bool("permissions", "cmd_get_market_list",
                             ServerSettings::__cmd_get_market_list);
    App::Me().Config().SetOption_bool("permissions", "cmd_get_market_offers",
                             ServerSettings::__cmd_get_market_offers);
    App::Me().Config().SetOption_bool("permissions", "cmd_get_market_recent_trades",
                             ServerSettings::__cmd_get_market_recent_trades);
    App::Me().Config().SetOption_bool("permissions", "cmd_get_nym_market_offers",
                             ServerSettings::__cmd_get_nym_market_offers);
    App::Me().Config().SetOption_bool("permissions", "transact_market_offer",
                             ServerSettings::__transact_market_offer);
    App::Me().Config().SetOption_bool("permissions", "transact_payment_plan",
                             ServerSettings::__transact_payment_plan);
    App::Me().Config().SetOption_bool("permissions", "transact_cancel_cron_item",
                             ServerSettings::__transact_cancel_cron_item);
    App::Me().Config().SetOption_bool("permissions", "transact_smart_contract",
                             ServerSettings::__transact_smart_contract);
    App::Me().Config().SetOption_bool("permissions", "cmd_trigger_clause",
                             ServerSettings::__cmd_trigger_clause);

    // Done Loading... Lets save any changes...
    if (!App::Me().Config().Save()) {
        Log::vError("%s: Error! Unable to save updated Config!!!\n", szFunc);
        OT_FAIL;
    }

    return true;
}

} // namespace opentxs
