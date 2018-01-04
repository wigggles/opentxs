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

#include "opentxs/stdafx.hpp"

#include "opentxs/client/OTME_too.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/Api.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/client/MadeEasy.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OTAPI_Func.hpp"
#include "opentxs/client/OT_ME.hpp"
#include "opentxs/client/SwigWrap.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"

#include <chrono>
#include <functional>
#include <sstream>

#define MASTER_SECTION "Master"
#define PAIRED_NODES_KEY "paired_nodes"
#define INTRODUCTION_SERVER_KEY "introduction_server_id"
#define PAIRED_SECTION_PREFIX "paired_node_"
#define BRIDGE_NYM_KEY "bridge_nym_id"
#define ADMIN_PASSWORD_KEY "admin_password"
#define OWNER_NYM_KEY "owner_nym_id"
#define NOTARY_ID_KEY "notary_id"
#define BACKUP_KEY "backup"
#define CONNECTED_KEY "connected"
#define DONE_KEY "done"
#define ISSUED_UNITS_KEY "issued_units"
#define ISSUED_UNIT_PREFIX_KEY "issued_unit_"
#define ASSET_ID_PREFIX_KEY "unit_definition_"
#define ACCOUNT_ID_PREFIX_KEY "account_id_"
#define NYM_REVISION_SECTION_PREFIX "nym_revision_"
#define RENAME_KEY "rename_started"
#define CONTACT_COUNT_KEY "contacts"
#define CONTACT_SECTION_PREFIX "contact_"
#define CONTACT_NYMID_KEY "nymid"
#define CONTACT_PAYMENTCODE_KEY "payment_code"
#define CONTACT_UPDATED_KEY "updated"
#define CONTACT_REVISION_KEY "revision"
#define CONTACT_LABEL_KEY "label"
#define CONTACT_REFRESH_DAYS 1
#define SERVER_NYM_INTERVAL 10
#define ALL_SERVERS "all"
#define PROCESS_INBOX_RETRIES 3

#define OT_METHOD "opentxs::OTME_too::"

namespace opentxs
{

const std::string OTME_too::DEFAULT_INTRODUCTION_SERVER =
    R"(-----BEGIN OT ARMORED SERVER CONTRACT-----
Version: Open Transactions 0.99.1-113-g2b3acf5
Comment: http://opentransactions.org

CAESI290b20xcHFmREJLTmJLR3RiN0NBa0ZodFRXVFVOTHFIRzIxGiNvdHVkd3p4cWF0UHh4
bmh4VFV3RUo3am5HenE2RkhGYTRraiIMU3Rhc2ggQ3J5cHRvKr8NCAESI290dWR3enhxYXRQ
eHhuaHhUVXdFSjdqbkd6cTZGSEZhNGtqGAIoATJTCAEQAiJNCAESIQI9MywLxxKfOtai26pj
JbxKtCCPhM/DbvX08iwbW2qYqhoga6Ccvp6CABGAFj/RdWNjtg5uzIRHT5Dn+fUzdAM9SUSA
AQCIAQA6vAwIARIjb3R1ZHd6eHFhdFB4eG5oeFRVd0VKN2puR3pxNkZIRmE0a2oaI290dXdo
ZzNwb2kxTXRRdVkzR3hwYWpOaXp5bmo0NjJ4Z2RIIAIymgQIARIjb3R1d2hnM3BvaTFNdFF1
WTNHeHBhak5penluajQ2MnhnZEgYAiABKAIyI290dWR3enhxYXRQeHhuaHhUVXdFSjdqbkd6
cTZGSEZhNGtqQl0IARJTCAEQAiJNCAESIQI9MywLxxKfOtai26pjJbxKtCCPhM/DbvX08iwb
W2qYqhoga6Ccvp6CABGAFj/RdWNjtg5uzIRHT5Dn+fUzdAM9SUSAAQCIAQAaBAgBEAJKiAEI
ARACGioIARAEGAIgASogZ6MtTp4aTEDLxFfhnsGo+Esp5B4hkgjWEejNPt5J6C0aKggBEAQY
AiACKiAhqJjWf2Ugqbg6z6ps59crAx9lHwTuT6Eq4x6JmkBlGBoqCAEQBBgCIAMqII2Vps1F
C2YUMbB4kE9XsHt1jrVY6pMPV6KWc5sH3VvTem0IARIjb3R1d2hnM3BvaTFNdFF1WTNHeHBh
ak5penluajQ2MnhnZEgYASAFKkDQLsszAol/Ih56MomuBKV8zpKaw5+ry7Kse1+5nPwJlP8f
72OAgTegBlmv31K4JgLVs52EKJTBpjnV+v0pxzUOem0IARIjb3R1ZHd6eHFhdFB4eG5oeFRV
d0VKN2puR3pxNkZIRmE0a2oYAyAFKkAJZ0LTVM+XBrGbRdiZsEQSbvwqg+mqGwHD5MQ+D4h0
fPQaUrdB6Pp/HM5veox02LBKg05hVNQ64tcU+LAxK+VHQuQDCAESI290clA2dDJXY2hYMjYz
ZVpiclRuVzZyY2FCZVNQb2VqSzJnGAIgAigCMiNvdHVkd3p4cWF0UHh4bmh4VFV3RUo3am5H
enE2RkhGYTRrajonCAESI290dXdoZzNwb2kxTXRRdVkzR3hwYWpOaXp5bmo0NjJ4Z2RISogB
CAEQAhoqCAEQBBgCIAEqIDpwlCrxHNWvvtFt6k8ocB5NBo7vjkGO/mRuSOQ/j/9WGioIARAE
GAIgAiog6Dw0+AWok4dENWWc/3qhykA7NNybWecqMGs5fL8KLLYaKggBEAQYAiADKiD+s/iq
37NrYI4/xdHOYtO/ocR0YqDVz09IaDNGVEdBtnptCAESI290clA2dDJXY2hYMjYzZVpiclRu
VzZyY2FCZVNQb2VqSzJnGAEgBSpATbHtakma53Na35Be+rGvW+z1H6EtkHlljv9Mo8wfies3
in9el1Ejb4BDbGCN5ABl3lQpfedZnR+VYv2X6Y1yBnptCAESI290dXdoZzNwb2kxTXRRdVkz
R3hwYWpOaXp5bmo0NjJ4Z2RIGAEgBSpAeptEmgdqgkGUcOJCqG0MsiChEREUdDzH/hRj877u
WDIHoRHsf/k5dCOHfDct4TDszasVhGFhRdNunpgQJcp0DULnAwgBEiNvdHd6ZWd1dTY3cENI
RnZhYjZyS2JYaEpXelNvdlNDTGl5URgCIAIoAjIjb3R1ZHd6eHFhdFB4eG5oeFRVd0VKN2pu
R3pxNkZIRmE0a2o6JwgBEiNvdHV3aGczcG9pMU10UXVZM0d4cGFqTml6eW5qNDYyeGdkSEqL
AQgBEAIaKwgBEAMYAiABKiEC5p36Ivxs4Wb6CjKTnDA1MFtX3Mx2UBlrmloSt+ffXz0aKwgB
EAMYAiACKiECtMkEo4xsefeevzrBb62ll98VYZy8PipgbrPWqGUNxQMaKwgBEAMYAiADKiED
W1j2DzOZemB9OOZ/pPrFroKDfgILYu2IOtiRFfi0vDB6bQgBEiNvdHd6ZWd1dTY3cENIRnZh
YjZyS2JYaEpXelNvdlNDTGl5URgBIAUqQJYd860/Ybh13GtW+grxWtWjjmzPifHE7bTlgUWl
3bX+ZuWNeEotA4yXQvFNog4PTAOF6dbvCr++BPGepBEUEEx6bQgBEiNvdHV3aGczcG9pMU10
UXVZM0d4cGFqTml6eW5qNDYyeGdkSBgBIAUqQH6GXnKCCDDgDvcSt8dLWuVMlr75zVkHy85t
tccoy2oLHNevDvKrLfUk/zuICyaSIvDy0Kb2ytOuh/O17yabxQ8yHQgBEAEYASISb3Quc3Rh
c2hjcnlwdG8ubmV0KK03MiEIARADGAEiFnQ1NGxreTJxM2w1ZGt3bnQub25pb24orTcyRwgB
EAQYASI8b3ZpcDZrNWVycXMzYm52cjU2cmgzZm5pZ2JuZjJrZWd1cm5tNWZpYnE1NWtqenNv
YW54YS5iMzIuaTJwKK03Op8BTWVzc2FnaW5nLW9ubHkgc2VydmVyIHByb3ZpZGVkIGZvciB0
aGUgY29udmllbmllbmNlIG9mIFN0YXNoIENyeXB0byB1c2Vycy4gU2VydmljZSBpcyBwcm92
aWRlZCBhcyBpcyB3aXRob3V0IHdhcnJhbnR5IG9mIGFueSBraW5kLCBlaXRoZXIgZXhwcmVz
c2VkIG9yIGltcGxpZWQuQiCK4L5cnecfUFz/DQyvAklKC2pTmWQtxt9olQS5/0hUHUptCAES
I290clA2dDJXY2hYMjYzZVpiclRuVzZyY2FCZVNQb2VqSzJnGAUgBSpA1/bep0NTbisZqYns
MCL/PCUJ6FIMhej+ROPk41604x1jeswkkRmXRNjzLlVdiJ/pQMxG4tJ0UQwpxHxrr0IaBA==

-----END OT ARMORED SERVER CONTRACT-----)";

OTME_too::Cleanup::Cleanup(std::atomic<bool>& run)
    : run_(run)
{
    run_.store(true);
}

OTME_too::Cleanup::~Cleanup() { run_.store(false); }

OTME_too::OTME_too(
    std::recursive_mutex& lock,
    api::Settings& config,
    api::ContactManager& contacts,
    OT_API& otapi,
    OTAPI_Exec& exec,
    const MadeEasy& madeEasy,
    const OT_ME& otme,
    api::client::Wallet& wallet,
    api::crypto::Encode& encoding,
    api::Identity& identity)
    : api_lock_(lock)
    , config_(config)
    , contacts_(contacts)
    , ot_api_(otapi)
    , exec_(exec)
    , made_easy_(madeEasy)
    , otme_(otme)
    , wallet_(wallet)
    , encoding_(encoding)
    , identity_(identity)
    , refreshing_(false)
    , shutdown_(false)
    , introduction_server_set_(false)
    , need_server_nyms_(false)
    , refresh_count_(0)
{
    scan_contacts();
    load_introduction_server();
}

std::pair<bool, std::size_t> OTME_too::accept_incoming(
    const rLock& lock[[maybe_unused]],
    const std::size_t max,
    const Identifier& accountID,
    ServerContext& context)
{
    std::pair<bool, std::size_t> output{false, 0};
    auto & [ success, remaining ] = output;
    const std::string account = String(accountID).Get();
    auto processInbox = ot_api_.CreateProcessInbox(accountID, context);
    auto& response = std::get<0>(processInbox);
    auto& inbox = std::get<1>(processInbox);

    if (false == bool(response)) {
        if (nullptr == inbox) {
            // This is a new account which has never instantiated an inbox.
            success = true;

            return output;
        }

        otErr << OT_METHOD << __FUNCTION__
              << ": Error instantiating processInbox for account: " << account
              << std::endl;

        return output;
    }

    const std::size_t items =
        (inbox->GetTransactionCount() >= 0) ? inbox->GetTransactionCount() : 0;
    const std::size_t count = (items > max) ? max : items;
    remaining = items - count;

    if (0 == count) {
        otInfo << OT_METHOD << __FUNCTION__
               << ": No items to accept in this account." << std::endl;
        success = true;

        return output;
    }

    for (std::size_t i = 0; i < count; i++) {
        auto transaction = inbox->GetTransactionByIndex(i);

        OT_ASSERT(nullptr != transaction);

        const TransactionNumber number = transaction->GetTransactionNum();

        if (transaction->IsAbbreviated()) {
            inbox->LoadBoxReceipt(number);
            transaction = inbox->GetTransaction(number);

            if (nullptr == transaction) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Unable to load item: " << number << std::endl;

                continue;
            }
        }

        const bool accepted = ot_api_.IncludeResponse(
            accountID, true, context, *transaction, *response);

        if (!accepted) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to accept item: " << number << std::endl;

            return output;
        }
    }

    const bool finalized =
        ot_api_.FinalizeProcessInbox(accountID, context, *response, *inbox);

    if (false == finalized) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to finalize response."
              << std::endl;

        return output;
    }

    const auto result = otme_.process_inbox(
        String(context.Server()).Get(),
        String(context.Nym()->ID()).Get(),
        account,
        String(*response).Get());
    success =
        (1 ==
         otme_.InterpretTransactionMsgReply(
             String(context.Server()).Get(),
             String(context.Nym()->ID()).Get(),
             account,
             "process_inbox",
             result));

    return output;
}

bool OTME_too::AcceptIncoming(
    const Identifier& nymID,
    const Identifier& accountID,
    const Identifier& serverID,
    const std::size_t max)
{
    rLock apiLock(api_lock_);
    auto context = wallet_.mutable_ServerContext(nymID, serverID);
    std::size_t remaining{1};
    std::size_t retries{PROCESS_INBOX_RETRIES};

    while (0 < remaining) {
        const auto attempt =
            accept_incoming(apiLock, max, accountID, context.It());
        const auto & [ success, unprocessed ] = attempt;
        remaining = unprocessed;

        if (false == success) {
            if (0 == retries) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Exceeded maximum retries." << std::endl;

                return false;
            }

            Utility utility(context.It(), ot_api_);
            const auto download = utility.getIntermediaryFiles(
                String(context.It().Server()).Get(),
                String(context.It().Nym()->ID()).Get(),
                String(accountID).Get(),
                true);

            if (false == download) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed to download account files." << std::endl;

                return false;
            } else {
                --retries;

                continue;
            }
        }

        if (0 != remaining) {
            otErr << OT_METHOD << __FUNCTION__ << ": Accepting " << remaining
                  << " more items." << std::endl;
        }
    }

    return true;
}

Identifier OTME_too::add_background_thread(BackgroundThread thread)
{
    Identifier output;

    if (shutdown_.load()) {
        return output;
    }

    std::unique_lock<std::mutex> lock(thread_lock_);

    Data nonce;
    encoding_.Nonce(32, nonce);
    output.CalculateDigest(nonce);
    auto& item = threads_[output];
    auto& running = std::get<0>(item);
    auto& handle = std::get<1>(item);
    auto& exitStatus = std::get<2>(item);

    OT_ASSERT(!handle);

    running.store(false);
    handle.reset(new std::thread(thread, &running, &exitStatus));

    return output;
}

void OTME_too::add_checknym_tasks(
    const nymAccountMap nyms,
    serverTaskMap& tasks)
{
    for (const auto nym : nyms) {
        const auto& serverID = nym.first;
        const auto& nymList = nym.second;
        const bool all = (ALL_SERVERS == serverID);

        if (all) {
            for (auto& it : tasks) {
                auto& server = it.second;
                server.second = nymList;
            }
        } else {
            tasks[serverID].second = nymList;
        }
    }
}

void OTME_too::build_account_list(serverTaskMap& output) const
{
    // Make sure no nyms, servers, or accounts are added or removed while
    // creating the list
    rLock apiLock(api_lock_);
    const auto serverList = wallet_.ServerList();
    const auto nymCount = exec_.GetNymCount();
    const auto accountCount = exec_.GetAccountCount();

    for (const auto server : serverList) {
        const auto& serverID = server.first;

        for (std::int32_t n = 0; n < nymCount; n++) {
            const auto nymID = exec_.GetNym_ID(n);

            if (exec_.IsNym_RegisteredAtServer(nymID, serverID)) {
                auto& tasks = output[serverID];
                auto& accounts = tasks.first;
                accounts.insert({nymID, {}});
            }
        }
    }

    if (!yield()) {
        return;
    }

    for (std::int32_t n = 0; n < accountCount; n++) {
        const auto accountID = exec_.GetAccountWallet_ID(n);
        const auto serverID = exec_.GetAccountWallet_NotaryID(accountID);
        const auto nymID = exec_.GetAccountWallet_NymID(accountID);

        auto& tasks = output[serverID];
        auto& accounts = tasks.first;
        auto& nym = accounts[nymID];
        nym.push_back(accountID);
    }

    apiLock.unlock();
    yield();
}

void OTME_too::build_nym_list(std::list<std::string>& output) const
{
    output.clear();

    // Make sure no nyms are added or removed while creating the list
    rLock apiLock(api_lock_);
    const auto nymCount = exec_.GetNymCount();

    for (std::int32_t n = 0; n < nymCount; n++) {
        output.push_back(exec_.GetNym_ID(n));
    }

    apiLock.unlock();
}

Messagability OTME_too::can_message(
    const std::string& senderNymID,
    const std::string& recipientContactID,
    std::string& server)
{
    auto senderNym = wallet_.Nym(Identifier(senderNymID));

    if (!senderNym) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load sender nym "
              << senderNymID << std::endl;

        return Messagability::MISSING_SENDER;
    }

    const bool canSign = senderNym->hasCapability(NymCapability::SIGN_MESSAGE);

    if (!canSign) {
        otErr << OT_METHOD << __FUNCTION__ << ": Sender nym " << senderNymID
              << " can not sign messages (no private key)." << std::endl;

        return Messagability::INVALID_SENDER;
    }

    const auto contact = contacts_.Contact(Identifier(recipientContactID));

    if (false == bool(contact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Recipient contact "
              << recipientContactID << " does not exist." << std::endl;

        return Messagability::MISSING_CONTACT;
    }

    const auto nyms = contact->Nyms();

    if (0 == nyms.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Recipient contact "
              << recipientContactID << " does not have a nym." << std::endl;

        return Messagability::CONTACT_LACKS_NYM;
    }

    std::shared_ptr<const Nym> recipientNym{nullptr};
    Identifier recipientNymID{};

    for (const auto& it : nyms) {
        recipientNym = wallet_.Nym(it);

        if (recipientNym) {
            recipientNymID = it;
            break;
        }
    }

    if (false == bool(recipientNym)) {
        for (const auto& id : nyms) {
            mailability(senderNymID, String(id).Get());
        }

        otErr << OT_METHOD << __FUNCTION__ << ": Recipient contact "
              << recipientContactID << " credentials not available."
              << std::endl;

        return Messagability::MISSING_RECIPIENT;
    }

    const auto claims = recipientNym->Claims();
    auto serverID = claims.PreferredOTServer();
    server = String(serverID).Get();

    if (server.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Recipient contact "
              << recipientContactID << ", nym " << String(recipientNymID)
              << ": credentials do not specify a server." << std::endl;
        mailability(senderNymID, String(recipientNymID).Get());

        return Messagability::NO_SERVER_CLAIM;
    }

    const bool registered = exec_.IsNym_RegisteredAtServer(senderNymID, server);

    if (!registered) {
        mailability(senderNymID, String(recipientNymID).Get());
        otErr << OT_METHOD << __FUNCTION__ << ": Sender nym " << senderNymID
              << " not registered on server " << server << std::endl;

        return Messagability::UNREGISTERED;
    }

    return Messagability::READY;
}

Messagability OTME_too::CanMessage(
    const std::string& sender,
    const std::string& recipient)
{
    std::string notUsed;

    return can_message(sender, recipient, notUsed);
}

bool OTME_too::check_nym_revision(
    const std::string& nymID,
    const std::string& server) const
{
    rLock apiLock(api_lock_);

    auto nym = wallet_.Nym(Identifier(nymID));

    if (!nym) {
        return false;
    }

    bool dontCare = false;
    String section = NYM_REVISION_SECTION_PREFIX;
    section.Concatenate(String(nymID));
    const String key(server);
    const std::int64_t local(nym->Revision());
    std::int64_t remote = 0;

    if (!config_.Check_long(section, key, remote, dontCare)) {
        return false;
    }

    return (local == remote);
}

bool OTME_too::check_server_registration(
    const std::string& nym,
    const std::string& server,
    const bool force,
    const bool publish) const
{
    if (!force) {
        const bool registered = exec_.IsNym_RegisteredAtServer(nym, server);

        if (!yield()) {
            return false;
        };

        if (registered) {
            return true;
        }
    }

    const bool updated = RegisterNym(nym, server, publish);

    if (!updated) {
        return false;
    }

    return update_nym_revision(nym, server);
}

void OTME_too::clean_background_threads()
{
    shutdown_.store(true);

    while (0 < threads_.size()) {
        for (auto it = threads_.begin(); it != threads_.end();) {
            auto& item = it->second;
            auto& running = std::get<0>(item);
            auto& handle = std::get<1>(item);

            if (!running.load()) {
                if (handle) {
                    if (handle->joinable()) {
                        handle->join();
                    }

                    handle.reset();
                }

                it = threads_.erase(it);
            } else {
                it++;
            }
        }
    }
}

bool OTME_too::do_i_download_server_nym() const
{
    if (need_server_nyms_.load()) {

        return true;
    }

    const bool output = (0 == (refresh_count_.load() % SERVER_NYM_INTERVAL));

    return output;
}

bool OTME_too::download_nym(
    const std::string& localNym,
    const std::string& remoteNym,
    const std::string& server) const
{
    std::string serverID;

    if (server.empty() && introduction_server_set_.load()) {
        serverID = String(introduction_server_).Get();
    } else {
        serverID = server;
    }

    const auto result = otme_.check_nym(serverID, localNym, remoteNym);

    if (!yield()) {
        return false;
    };

    return (1 == otme_.VerifyMessageSuccess(result));
}

void OTME_too::establish_mailability(
    const std::string& sender,
    const std::string& recipient,
    std::atomic<bool>* running,
    std::atomic<bool>* exitStatus)
{
    OT_ASSERT(nullptr != running)
    OT_ASSERT(nullptr != exitStatus)

    Cleanup threadStatus(*running);
    exitStatus->store(false);

    std::unique_lock<std::mutex> lock(messagability_lock_);
    const std::pair<std::string, std::string> key{sender, recipient};
    auto& runningMessagability = messagability_map_[key];
    bool previous = runningMessagability.load();

    if (previous) {
        otInfo << OT_METHOD << __FUNCTION__ << ": Thread already running."
               << std::endl;

        return;
    }

    Cleanup messagabilityStatus(runningMessagability);
    lock.unlock();
    auto recipientNym = wallet_.Nym(Identifier(recipient));

    if (!recipientNym) {
        otErr << OT_METHOD << __FUNCTION__ << ": Searching for recipient nym "
              << recipient << std::endl;

        FindNym(recipient, "");

        return;
    }

    const auto claims = recipientNym->Claims();
    const auto serverID = claims.PreferredOTServer();
    const std::string server = String(serverID).Get();

    if (server.empty()) {
        FindNym(recipient, "");

        return;
    }

    const auto serverContract = wallet_.Server(Identifier(server));

    if (false == bool(serverContract)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Searching for server contract"
              << server << std::endl;

        FindServer(server);

        return;
    }

    const bool registered = exec_.IsNym_RegisteredAtServer(sender, server);

    bool exit = false;

    if (false == registered) {
        otErr << OT_METHOD << __FUNCTION__ << ": Registering on target server "
              << server << std::endl;

        exit = RegisterNym(sender, server, true);
    }

    exitStatus->store(exit);
}

std::set<std::string> OTME_too::extract_message_servers(
    const std::string& nymID) const
{
    const auto nym = wallet_.Nym(Identifier(nymID));

    if (!nym) {
        return {};
    }

    const auto group = nym->Claims().Group(
        proto::CONTACTSECTION_COMMUNICATION, proto::CITEMTYPE_OPENTXS);

    if (false == bool(group)) {

        return {};
    }

    return unique_servers(*group);
}

void OTME_too::fill_registered_servers(
    std::string& introductionNym,
    std::set<std::string>& serverList,
    std::list<std::pair<std::string, std::string>>& serverNymList) const
{
    serverTaskMap accounts;
    build_account_list(accounts);
    std::string introductionServer{};

    if (introduction_server_set_.load()) {
        introductionServer = String(introduction_server_).Get();
    }

    for (const auto server : accounts) {
        const auto& serverID = server.first;
        const auto& tasks = server.second;
        const auto& accountMap = tasks.first;
        const auto it = accountMap.begin();

        if (0 == accountMap.size()) {
            continue;
        }

        const auto& nymID = it->first;

        if (nymID.empty()) {
            continue;
        }

        if (serverID == introductionServer) {
            introductionNym = nymID;

            continue;
        }

        if (0 == serverList.count(serverID)) {
            serverNymList.push_back({serverID, nymID});
            serverList.insert(serverID);
        }
    }
}

void OTME_too::fill_viable_servers(
    std::list<std::pair<std::string, std::string>>& serverList) const
{
    std::set<std::string> servers;
    std::string introductionNym;
    std::string introductionServer{};

    if (introduction_server_set_.load()) {
        introductionServer = String(introduction_server_).Get();
    }

    fill_registered_servers(introductionNym, servers, serverList);

    if ((!introductionServer.empty()) && (!introductionNym.empty())) {
        serverList.push_back({introductionServer, introductionNym});
    }
}

void OTME_too::find_nym(
    const std::string& remoteNymID,
    const std::string& serverIDhint,
    std::atomic<bool>* running,
    std::atomic<bool>* exitStatus) const
{
    OT_ASSERT(nullptr != running)
    OT_ASSERT(nullptr != exitStatus)

    Cleanup threadStatus(*running);
    exitStatus->store(false);
    std::list<std::pair<std::string, std::string>> serverList;
    fill_viable_servers(serverList);

    if (!serverIDhint.empty()) {
        for (auto it = serverList.begin(); it != serverList.end();) {
            const auto& serverID = it->first;
            const auto& nymID = it->second;

            if (serverIDhint == serverID) {

            } else {
                serverList.push_front({serverID, nymID});
                it = serverList.erase(it);
            }
        }
    }

    if (0 == serverList.size()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": No servers available to search for nym " << remoteNymID
              << std::endl;
    }

    for (const auto& it : serverList) {
        const auto& serverID = it.first;
        const auto& nymID = it.second;

        const auto response = otme_.check_nym(serverID, nymID, remoteNymID);
        const bool found = (1 == otme_.VerifyMessageSuccess(response));

        if (found) {
            otErr << OT_METHOD << __FUNCTION__ << ": nym " << remoteNymID
                  << " found on " << serverID << "." << std::endl;
            exitStatus->store(true);

            break;
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": nym " << remoteNymID
                  << " not found on " << serverID << "." << std::endl;
        }

        if (shutdown_.load()) {
            break;
        }
    }
}

void OTME_too::find_nym_if_necessary(const std::string& nymID, Identifier& task)
{
    auto nym = wallet_.Nym(Identifier(nymID));

    if (nym) {
        return;
    }

    const auto status = Status(task);

    switch (status) {
        case ThreadStatus::ERROR:
        case ThreadStatus::FINISHED_SUCCESS:
        case ThreadStatus::FINISHED_FAILED: {
            task = FindNym(nymID, "");
        } break;
        default: {
        }
    }
}

void OTME_too::find_server(
    const std::string& remoteServerID,
    std::atomic<bool>* running,
    std::atomic<bool>* exitStatus) const
{
    OT_ASSERT(nullptr != running)
    OT_ASSERT(nullptr != exitStatus)

    Cleanup threadStatus(*running);
    exitStatus->store(false);
    std::list<std::pair<std::string, std::string>> serverList;
    fill_viable_servers(serverList);

    for (const auto& it : serverList) {
        const auto& serverID = it.first;
        const auto& nymID = it.second;

        const auto response =
            otme_.retrieve_contract(serverID, nymID, remoteServerID);
        const bool found = (1 == otme_.VerifyMessageSuccess(response));

        if (found) {
            otErr << __FUNCTION__ << ": server " << remoteServerID
                  << " found on " << serverID << "." << std::endl;
            exitStatus->store(true);

            break;
        }

        if (shutdown_.load()) {
            break;
        }
    }
}

Identifier OTME_too::FindNym(
    const std::string& nymID,
    const std::string& serverHint)
{
    OTME_too::BackgroundThread thread =
        [=](std::atomic<bool>* running, std::atomic<bool>* exit) -> void {
        find_nym(nymID, serverHint, running, exit);
    };

    return add_background_thread(thread);
}

Identifier OTME_too::FindServer(const std::string& serverID)
{
    OTME_too::BackgroundThread thread =
        [=](std::atomic<bool>* running, std::atomic<bool>* exit) -> void {
        find_server(serverID, running, exit);
    };

    return add_background_thread(thread);
}

void OTME_too::get_admin(
    const Identifier& nymID,
    const Identifier& serverID,
    const std::string& password) const
{
    rLock lock(api_lock_);
    bool success{false};

    {
        OTAPI_Func action(
            REQUEST_ADMIN, wallet_, nymID, serverID, exec_, ot_api_, password);
        action.Run();

        if (SendResult::VALID_REPLY == action.LastSendResult()) {
            auto reply = action.Reply();

            OT_ASSERT(reply)

            success = reply->m_bSuccess;
        }
    }

    auto mContext = wallet_.mutable_ServerContext(nymID, serverID);
    auto& context = mContext.It();
    context.SetAdminAttempted();

    if (success) {
        otErr << OT_METHOD << __FUNCTION__ << ": Got admin on server "
              << String(serverID) << std::endl;
        context.SetAdminSuccess();
    }
}

std::string OTME_too::get_introduction_server(const Lock& lock) const
{
    bool keyFound = false;
    String serverID;
    rLock apiLock(api_lock_);
    const bool config = config_.Check_str(
        MASTER_SECTION, INTRODUCTION_SERVER_KEY, serverID, keyFound);

    if (!config || !keyFound || !serverID.Exists()) {

        return import_default_introduction_server(lock);
    }

    return serverID.Get();
}

std::time_t OTME_too::get_time(const std::string& alias) const
{
    std::time_t output = 0;

    try {
        output = std::stoi(alias);
    } catch (std::invalid_argument) {
        output = 0;
    } catch (std::out_of_range) {
        output = 0;
    }

    return output;
}

const Identifier& OTME_too::GetIntroductionServer() const
{
    if (introduction_server_set_.load()) {

        return introduction_server_;
    }

    load_introduction_server();

    return introduction_server_;
}

std::string OTME_too::import_default_introduction_server(const Lock& lock) const
{
    return set_introduction_server(lock, OTME_too::DEFAULT_INTRODUCTION_SERVER);
}

std::string OTME_too::ImportNym(const std::string& input) const
{
    const auto serialized =
        proto::StringToProto<proto::CredentialIndex>(String(input.c_str()));
    const auto nym = wallet_.Nym(serialized);

    if (nym) {
        return String(nym->ID()).Get();
    }

    return {};
}

std::uint64_t OTME_too::legacy_contact_count() const
{
    std::int64_t result = 0;
    bool notUsed = false;
    rLock apiLock(api_lock_);
    config_.Check_long(MASTER_SECTION, CONTACT_COUNT_KEY, result, notUsed);

    if (1 > result) {
        return 0;
    }

    return result;
}

void OTME_too::load_introduction_server() const
{
    Lock lock(introduction_server_lock_);

    introduction_server_ =
        Identifier(String(get_introduction_server(lock).data()));

    if (false == introduction_server_.empty()) {
        introduction_server_set_.store(true);
    }
}

void OTME_too::mailability(
    const std::string& sender,
    const std::string& recipient)
{
    OTME_too::BackgroundThread thread =
        [=](std::atomic<bool>* running, std::atomic<bool>* exit) -> void {
        establish_mailability(sender, recipient, running, exit);
    };

    add_background_thread(thread);
}

void OTME_too::message_contact(
    const std::string& server,
    const std::string& senderNymID,
    const std::string& contactID,
    const std::string& message,
    std::atomic<bool>* running,
    std::atomic<bool>* exitStatus)
{
    OT_ASSERT(nullptr != running)
    OT_ASSERT(nullptr != exitStatus)

    Cleanup threadStatus(*running);
    exitStatus->store(false);
    const auto contact = contacts_.Contact(Identifier(contactID));

    if (false == bool(contact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Contact does not exist."
              << std::endl;

        return;
    }

    const auto nyms = contact->Nyms();

    if (0 == nyms.size()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Contact does not have a nym id." << std::endl;

        return;
    }

    std::shared_ptr<const Nym> recipientNym{nullptr};
    Identifier recipientNymID{};

    for (const auto& it : nyms) {
        recipientNym = wallet_.Nym(it);

        if (recipientNym) {
            recipientNymID = it;
            break;
        }
    }

    if (false == bool(recipientNym)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Recipient nym credentials not found." << std::endl;

        return;
    }

    const auto result = otme_.send_user_msg(
        server, senderNymID, String(recipientNymID).Get(), message);
    const bool success = (1 == otme_.VerifyMessageSuccess(result));
    exitStatus->store(success);

    if (false == success) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to message nym "
              << String(recipientNymID) << " on server " << server << std::endl;
    }
}

Identifier OTME_too::MessageContact(
    const std::string& senderNymID,
    const std::string& contactID,
    const std::string& message)
{
    std::string server;
    const auto messagability = can_message(senderNymID, contactID, server);

    if (Messagability::READY != messagability) {
        otWarn << OT_METHOD << __FUNCTION__ << ": Unable to message ("
               << std::to_string(static_cast<std::int8_t>(messagability)) << ")"
               << std::endl;

        return {};
    }

    OTME_too::BackgroundThread thread =
        [=](std::atomic<bool>* running, std::atomic<bool>* exit) -> void {
        message_contact(server, senderNymID, contactID, message, running, exit);
    };

    return add_background_thread(thread);
}

bool OTME_too::need_to_refresh(const std::string& serverID)
{
    Lock lock(refresh_interval_lock_);

    auto it = refresh_interval_.find(serverID);

    if (refresh_interval_.end() == it) {
        // This server has not been configured for reduced updates

        return true;
    }

    auto& interval = it->second;

    if (0 == interval) {
        // A value of zero means update every loop

        return true;
    }

    return (0 == (refresh_count_.load() % interval));
}

std::string OTME_too::obtain_server_id(
    const std::string& ownerNym,
    const Nym& bridgeNym) const
{
    std::string output = String(bridgeNym.Claims().PreferredOTServer()).Get();

    if (output.empty()) {
        download_nym(ownerNym, String(bridgeNym.ID()).Get(), "");
    }

    return output;
}

std::string OTME_too::obtain_server_id(const std::string& nymID) const
{
    auto nym = wallet_.Nym(Identifier(nymID));

    if (!nym) {

        return {};
    }

    return String(nym->Claims().PreferredOTServer()).Get();
}

void OTME_too::parse_contact_section(const std::uint64_t index)
{
    rLock apiLock(api_lock_);
    bool notUsed = false;
    String nym;
    String name;
    String paymentCode;
    std::int64_t checked = 0;
    std::int64_t version = 0;
    String section = CONTACT_SECTION_PREFIX;
    String key = std::to_string(index).c_str();
    section.Concatenate(key);
    config_.Check_str(section, CONTACT_NYMID_KEY, nym, notUsed);
    config_.Check_long(section, CONTACT_UPDATED_KEY, checked, notUsed);
    config_.Check_long(section, CONTACT_REVISION_KEY, version, notUsed);
    config_.Check_str(section, CONTACT_LABEL_KEY, name, notUsed);
    config_.Check_str(section, CONTACT_PAYMENTCODE_KEY, paymentCode, notUsed);
    const bool ready = nym.Exists();

    if (!ready) {
        return;
    }

    if (0 > version) {
        version = 0;
    }

    const Identifier nymID(nym);
    auto contactID = contacts_.ContactID(nymID);

    if (contactID.empty()) {
        contacts_.NewContact(name.Get(), nymID, PaymentCode(paymentCode.Get()));
    } else {
        auto contactEditor = contacts_.mutable_Contact(contactID);

        OT_ASSERT(contactEditor);

        auto& contact = contactEditor->It();

        if (contact.Label().empty()) {
            contact.SetLabel(name.Get());
        }

        if (contact.PaymentCode().empty()) {
            contact.AddPaymentCode(PaymentCode(paymentCode.Get()), true);
        }
    }
}

bool OTME_too::publish_server_registration(
    const std::string& nymID,
    const std::string& server,
    const bool forcePrimary) const
{
    auto nym = wallet_.mutable_Nym(Identifier(nymID));

    return nym.AddPreferredOTServer(server, forcePrimary);
}

void OTME_too::refresh_contacts(nymAccountMap& nymsToCheck)
{
    for (const auto& it : contacts_.ContactList()) {
        const auto& contactID = it.first;
        otInfo << OT_METHOD << __FUNCTION__
               << ": Considering contact: " << contactID << std::endl;

        const auto contact = contacts_.Contact(Identifier(contactID));

        OT_ASSERT(contact);
        const auto now = std::time(nullptr);
        const std::chrono::seconds interval(now - contact->LastUpdated());
        const std::chrono::hours limit(24 * CONTACT_REFRESH_DAYS);
        const auto nymList = contact->Nyms();

        if (nymList.empty()) {
            otInfo << OT_METHOD << __FUNCTION__
                   << ": No nyms associated with this contact." << std::endl;

            continue;
        }

        for (const auto& it : nymList) {
            const auto nym = wallet_.Nym(it);
            const std::string nymID = String(it).Get();
            otInfo << OT_METHOD << __FUNCTION__
                   << ": Considering nym: " << nymID << std::endl;

            if (nym) {
                contacts_.Update(nym->asPublicNym());
            } else {
                otInfo << OT_METHOD << __FUNCTION__
                       << ": We don't have credentials for this nym. "
                       << " Will search on all servers." << std::endl;
                nymsToCheck[ALL_SERVERS].push_back(nymID);

                continue;
            }

            if (interval > limit) {
                otInfo << OT_METHOD << __FUNCTION__
                       << ": Hours since last update (" << interval.count()
                       << ") exceeds the limit (" << limit.count() << ")"
                       << std::endl;
                const auto servers = extract_message_servers(nymID);

                for (const auto& server : servers) {
                    otInfo << OT_METHOD << __FUNCTION__
                           << ": Will download nym " << nymID << " from server "
                           << server << std::endl;
                    nymsToCheck[server].push_back(nymID);
                }
            } else {
                otInfo << OT_METHOD << __FUNCTION__
                       << ": No need to update this nym." << std::endl;
            }
        }
    }
}

void OTME_too::refresh_thread()
{
    Cleanup cleanup(refreshing_);
    otInfo << OT_METHOD << __FUNCTION__ << ": Starting refresh loop."
           << std::endl;
    serverTaskMap accounts;
    build_account_list(accounts);
    otInfo << OT_METHOD << __FUNCTION__ << ": Account list created."
           << std::endl;
    nymAccountMap nymsToCheck;
    refresh_contacts(nymsToCheck);
    otInfo << OT_METHOD << __FUNCTION__ << ": Checknym task list created."
           << std::endl;
    add_checknym_tasks(nymsToCheck, accounts);
    otInfo << OT_METHOD << __FUNCTION__ << ": Server operation list finished."
           << std::endl;

    for (const auto server : accounts) {
        bool updateServerNym = do_i_download_server_nym();
        const auto& serverID = server.first;
        const auto& tasks = server.second;
        const auto& accountList = tasks.first;
        const auto& checkNym = tasks.second;
        bool nymsChecked = false;

        if (false == need_to_refresh(serverID)) {
            otInfo << OT_METHOD << __FUNCTION__
                   << ": Skipping update for server " << serverID << std::endl;

            continue;
        } else {
            otInfo << OT_METHOD << __FUNCTION__ << ": Updating server "
                   << serverID << std::endl;
        }

        for (const auto nym : accountList) {
            if (false == yield()) {
                return;
            }

            const auto& nymID = nym.first;
            otInfo << OT_METHOD << __FUNCTION__ << ": Refreshing nym " << nymID
                   << " on " << serverID << std::endl;
            bool needAdmin{false};
            std::string password{""};
            auto context =
                wallet_.ServerContext(Identifier(nymID), Identifier(serverID));

            if (context) {
                needAdmin = context->HaveAdminPassword() &&
                            (false == context->isAdmin());
                password = context->AdminPassword();
            }

            if (needAdmin) {
                get_admin(Identifier(nymID), Identifier(serverID), password);
            }

            if (updateServerNym) {
                otInfo << OT_METHOD << __FUNCTION__
                       << ": Downloading updated server nym." << std::endl;

                auto contract = wallet_.Server(Identifier(serverID));

                if (contract) {
                    const auto& serverNymID = contract->Nym()->ID();
                    const auto result = otme_.check_nym(
                        serverID, nymID, String(serverNymID).Get());
                    // If multiple nyms are registered on this server, we only
                    // need to successfully download the nym once.
                    updateServerNym = (1 != otme_.VerifyMessageSuccess(result));

                    if (updateServerNym) {
                        otInfo << OT_METHOD << __FUNCTION__
                               << ": Check nym for server nym "
                               << String(serverNymID) << " failed."
                               << std::endl;
                        otme_.register_nym(serverID, nymID);
                    }
                } else {
                    OT_FAIL;
                }
            }

            bool notUsed = false;
            otInfo << OT_METHOD << __FUNCTION__ << ": Downloading nymbox."
                   << std::endl;
            const auto retrieve =
                made_easy_.retrieve_nym(serverID, nymID, notUsed, true);

            if (1 != retrieve) {
                otInfo << OT_METHOD << __FUNCTION__
                       << ": Downloading nymbox failed (" << retrieve << ")"
                       << std::endl;
                otme_.register_nym(serverID, nymID);
            }

            // If the nym's credentials have been updated since the last time
            // it was registered on the server, upload the new credentials
            if (false == check_nym_revision(nymID, serverID)) {
                otInfo << OT_METHOD << __FUNCTION__
                       << ": Uploading new credentials to server." << std::endl;
                check_server_registration(nymID, serverID, true, false);
            }

            for (auto& account : nym.second) {
                if (!yield()) {
                    return;
                }

                otInfo << OT_METHOD << __FUNCTION__ << ": Downloading account "
                       << account << std::endl;
                made_easy_.retrieve_account(serverID, nymID, account, true);
            }

            if (!nymsChecked) {
                for (const auto& nym : checkNym) {
                    otInfo << OT_METHOD << __FUNCTION__ << ": Downloading nym "
                           << nym << std::endl;
                    made_easy_.check_nym(serverID, nymID, nym);

                    if (!yield()) {
                        return;
                    }
                }

                nymsChecked = true;
            }
        }
    }

    refresh_count_++;
    otInfo << OT_METHOD << __FUNCTION__ << ": Resending pending peer requests."
           << std::endl;
    resend_peer_requests();
    refreshing_.store(false);
    otInfo << OT_METHOD << __FUNCTION__ << ": Refresh complete." << std::endl;
}

void OTME_too::Refresh(const std::string&)
{
    const auto refreshing = refreshing_.exchange(true);

    if (!refreshing) {
        if (refresh_thread_) {
            refresh_thread_->join();
            refresh_thread_.reset();
        }

        refresh_thread_.reset(new std::thread(&OTME_too::refresh_thread, this));
    }
}

void OTME_too::register_nym(
    const std::string& nymID,
    const std::string& server,
    std::atomic<bool>* running,
    std::atomic<bool>* exitStatus)
{
    OT_ASSERT(nullptr != running)
    OT_ASSERT(nullptr != exitStatus)

    Cleanup threadStatus(*running);
    exitStatus->store(false);
    const auto result = otme_.register_nym(server, nymID);
    const bool registered = (1 == otme_.VerifyMessageSuccess(result));
    exitStatus->store(registered);
}

void OTME_too::RegisterIntroduction(const Identifier& nymID) const
{
    const auto& serverID = GetIntroductionServer();

    if (serverID.empty()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Introduction server unavailable" << std::endl;

        return;
    }

    const std::string nym = String(nymID).Get();
    const std::string server = String(serverID).Get();

    if (exec_.IsNym_RegisteredAtServer(nym, server)) {

        return;
    }

    RegisterNym(nym, server, true);
}

bool OTME_too::RegisterNym(
    const std::string& nymID,
    const std::string& server,
    const bool setContactData) const
{
    if (setContactData) {
        publish_server_registration(nymID, server, false);
    }

    const auto result = otme_.register_nym(server, nymID);
    const bool registered = (1 == otme_.VerifyMessageSuccess(result));

    return registered;
}

Identifier OTME_too::RegisterNym_async(
    const std::string& nymID,
    const std::string& server,
    const bool setContactData)
{
    if (setContactData) {
        publish_server_registration(nymID, server, false);
    }

    OTME_too::BackgroundThread thread =
        [=](std::atomic<bool>* running, std::atomic<bool>* exit) -> void {
        register_nym(nymID, server, running, exit);
    };

    return add_background_thread(thread);
}

std::uint64_t OTME_too::RefreshCount() const { return refresh_count_.load(); }

void OTME_too::resend_bailment(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    const auto result = otme_.initiate_bailment(
        request.bailment().serverid(),
        request.initiator(),
        request.recipient(),
        request.bailment().unitid());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_bailment_notification(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    const auto result = otme_.notify_bailment(
        request.pendingbailment().serverid(),
        request.initiator(),
        request.recipient(),
        request.pendingbailment().unitid(),
        request.pendingbailment().txid());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_connection_info(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    auto server = request.server();

    if (server.empty()) {
        otErr << __FUNCTION__ << ": This request was saved without the server "
              << "id. Attempting lookup based on recipient nym." << std::endl;

        server = obtain_server_id(request.recipient());
    }

    const auto result = otme_.request_connection(
        server,
        request.initiator(),
        request.recipient(),
        request.connectioninfo().type());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_outbailment(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    const auto result = otme_.initiate_outbailment(
        request.outbailment().serverid(),
        request.initiator(),
        request.recipient(),
        request.outbailment().unitid(),
        request.outbailment().amount(),
        request.outbailment().instructions());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_store_secret(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    auto server = request.server();

    if (server.empty()) {
        otErr << __FUNCTION__ << ": This request was saved without the server "
              << "id. Attempting lookup based on recipient nym." << std::endl;

        server = obtain_server_id(request.recipient());
    }

    const auto result = otme_.store_secret(
        server,
        request.initiator(),
        request.recipient(),
        request.storesecret().type(),
        request.storesecret().primary(),
        request.storesecret().secondary());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_peer_request(
    const Identifier& nymID,
    const Identifier& requestID) const
{
    rLock apiLock(api_lock_);
    std::time_t notUsed;

    auto request = wallet_.PeerRequest(
        nymID, requestID, StorageBox::SENTPEERREQUEST, notUsed);

    if (!request) {
        return;
    }

    switch (request->type()) {
        case proto::PEERREQUEST_BAILMENT: {
            resend_bailment(nymID, *request);
        } break;
        case proto::PEERREQUEST_OUTBAILMENT: {
            resend_outbailment(nymID, *request);
        } break;
        case proto::PEERREQUEST_PENDINGBAILMENT: {
            resend_bailment_notification(nymID, *request);
        } break;
        case proto::PEERREQUEST_CONNECTIONINFO: {
            resend_connection_info(nymID, *request);
        } break;
        case proto::PEERREQUEST_STORESECRET: {
            resend_store_secret(nymID, *request);
        } break;
        default: {
        }
    }

    apiLock.unlock();
}

void OTME_too::resend_peer_requests() const
{
    std::list<std::string> nyms;
    build_nym_list(nyms);
    const std::string empty = std::to_string(std::time_t(0));
    const std::time_t now = std::time(nullptr);
    const std::chrono::hours limit(24);

    for (const auto& nym : nyms) {
        const Identifier nymID(nym);
        const auto sent = wallet_.PeerRequestSent(nymID);

        for (const auto& request : sent) {
            const auto& id = request.first;
            const auto& alias = request.second;

            if (alias == empty) {
                otErr << __FUNCTION__ << ": Timestamp for peer request " << id
                      << " is not set. Setting now." << std::endl;

                wallet_.PeerRequestUpdate(
                    nymID, Identifier(id), StorageBox::SENTPEERREQUEST);
            } else {
                std::chrono::seconds interval(now - get_time(alias));

                if (interval > limit) {
                    otInfo << __FUNCTION__ << ": request " << id << " is "
                           << interval.count() << " seconds old. Resending."
                           << std::endl;
                    resend_peer_request(nymID, Identifier(id));
                } else {
                    otInfo << __FUNCTION__ << ": request " << id << " is "
                           << interval.count() << " seconds old. It's fine."
                           << std::endl;
                }
            }
        }
    }
}

std::string OTME_too::set_introduction_server(
    const Lock& lock,
    const std::string& contract) const
{

    std::string id = exec_.AddServerContract(contract);

    if (!yield()) {
        return {};
    }

    if (!id.empty()) {
        OT_ASSERT(verify_lock(lock, introduction_server_lock_));

        introduction_server_ = Identifier(id);
        introduction_server_set_.store(true);

        bool dontCare = false;
        rLock apiLock(api_lock_);
        const bool set = config_.Set_str(
            MASTER_SECTION, INTRODUCTION_SERVER_KEY, String(id), dontCare);

        if (!set) {
            id.clear();
        } else {
            config_.Save();
        }
    }

    return id;
}

std::string OTME_too::SetIntroductionServer(const std::string& contract) const
{
    Lock lock(introduction_server_lock_);

    return set_introduction_server(lock, contract);
}

void OTME_too::SetInterval(
    const std::string& server,
    const std::uint64_t interval) const
{
    Lock lock(refresh_interval_lock_);

    refresh_interval_[server] = interval;
}

void OTME_too::scan_contacts()
{
    const auto contactCount = legacy_contact_count();

    if (0 == contactCount) {

        return;
    }

    for (std::uint64_t n = 0; n < contactCount; n++) {
        parse_contact_section(n);
    }

    rLock apiLock(api_lock_);
    bool notUsed = false;
    config_.Set_long(MASTER_SECTION, CONTACT_COUNT_KEY, 0, notUsed);
    config_.Save();
}

void OTME_too::Shutdown()
{
    clean_background_threads();

    while (refreshing_.load()) {
        Log::Sleep(std::chrono::milliseconds(250));
    }

    if (refresh_thread_) {
        refresh_thread_->join();
        refresh_thread_.reset();
    }
}

ThreadStatus OTME_too::Status(const Identifier& thread)
{
    if (shutdown_.load()) {
        return ThreadStatus::SHUTDOWN;
    }

    std::unique_lock<std::mutex> lock(thread_lock_);

    auto it = threads_.find(thread);

    if (threads_.end() == it) {
        return ThreadStatus::ERROR;
    }

    const auto& status = std::get<0>(it->second);
    const bool exit_status = std::get<2>(it->second);

    if (status.load()) {
        return ThreadStatus::RUNNING;
    }

    const auto& handle = std::get<1>(it->second);

    OT_ASSERT(handle);

    if (handle->joinable()) {
        handle->join();
    }

    threads_.erase(it);

    if (exit_status) {

        return ThreadStatus::FINISHED_SUCCESS;
    }

    return ThreadStatus::FINISHED_FAILED;
}

std::set<std::string> OTME_too::unique_servers(const ContactGroup& group) const
{
    std::set<std::string> output;

    for (const auto& it : group) {
        OT_ASSERT(it.second);

        const auto& item = *it.second;
        output.insert(item.Value());
    }

    return output;
}

bool OTME_too::update_nym_revision(
    const std::string& nymID,
    const std::string& server) const
{
    rLock apiLock(api_lock_);

    auto nym = wallet_.Nym(Identifier(nymID));

    if (!nym) {
        return false;
    }

    bool dontCare = false;
    String section = NYM_REVISION_SECTION_PREFIX;
    section.Concatenate(String(nymID));
    const String key(server);
    const auto& revision = nym->Revision();

    if (!config_.Set_long(section, key, revision, dontCare)) {

        return false;
    }

    return config_.Save();
}

bool OTME_too::verify_lock(const Lock& lock, const std::mutex& mutex) const
{
    if (lock.mutex() != &mutex) {
        otErr << __FUNCTION__ << ": Incorrect mutex." << std::endl;

        return false;
    }

    if (false == lock.owns_lock()) {
        otErr << __FUNCTION__ << ": Lock not owned." << std::endl;

        return false;
    }

    return true;
}

bool OTME_too::yield() const
{
    Log::Sleep(std::chrono::milliseconds(50));

    return !shutdown_.load();
}

OTME_too::~OTME_too() { Shutdown(); }
}  // namespace opentxs
