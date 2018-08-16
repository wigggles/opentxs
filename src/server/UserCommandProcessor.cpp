// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "UserCommandProcessor.hpp"

#include "opentxs/api/server/Manager.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Legacy.hpp"
#include "opentxs/api/Wallet.hpp"
#if OT_CASH
#include "opentxs/cash/Mint.hpp"
#endif  // OT_CASH
#include "opentxs/client/NymData.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/contract/basket/BasketContract.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/script/OTParty.hpp"
#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/trade/OTMarket.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"

#include "Macros.hpp"
#include "MainFile.hpp"
#include "Notary.hpp"
#include "Server.hpp"
#include "ReplyMessage.hpp"
#include "ServerSettings.hpp"
#include "Transactor.hpp"

#include <cinttypes>
#include <memory>
#include <set>
#include <string>

#define OT_METHOD "opentxs::UserCommandProcessor::"
#define MAX_UNUSED_NUMBERS 50
#define ISSUE_NUMBER_BATCH 100
#define NYMBOX_DEPTH 0
#define INBOX_DEPTH 1
#define OUTBOX_DEPTH 2

namespace opentxs::server
{
UserCommandProcessor::FinalizeResponse::FinalizeResponse(
    const api::Wallet& wallet,
    const Nym& nym,
    ReplyMessage& reply,
    Ledger& ledger)
    : wallet_{wallet}
    , nym_(nym)
    , reply_(reply)
    , ledger_(ledger)
    , response_(nullptr)
    , counter_(0)
{
}

OTTransaction* UserCommandProcessor::FinalizeResponse::Release()
{
    counter_++;

    return response_.release();
}

OTTransaction* UserCommandProcessor::FinalizeResponse::Response()
{
    return response_.get();
}

void UserCommandProcessor::FinalizeResponse::SetResponse(
    OTTransaction* response)
{
    response_.reset(response);

    if (false == bool(response_)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to construct response transaction." << std::endl;

        OT_FAIL;
    }
}

UserCommandProcessor::FinalizeResponse::~FinalizeResponse()
{
    if ((false == bool(response_)) && (0 == counter_)) {
        response_.reset(OTTransaction::GenerateTransaction(
            wallet_,
            ledger_,
            OTTransaction::error_state,
            originType::not_applicable,
            0));

        if (false == response_->SignContract(nym_)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to sign response transaction." << std::endl;

            OT_FAIL;
        }

        if (false == response_->SaveContract()) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to serialize response transaction." << std::endl;

            OT_FAIL;
        }

        if (false == ledger_.AddTransaction(*response_.release())) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to add response transaction to response ledger."
                  << std::endl;

            OT_FAIL;
        }
    }

    ledger_.ReleaseSignatures();

    if (false == ledger_.SignContract(nym_)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to sign response ledger." << std::endl;

        OT_FAIL;
    }

    if (false == ledger_.SaveContract()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to serialize response ledger." << std::endl;

        OT_FAIL;
    }

    reply_.SetPayload(String(ledger_));
    otWarn << OT_METHOD << __FUNCTION__ << ": "
           << reply_.Context().AvailableNumbers() << " numbers available."
           << std::endl;
    otWarn << OT_METHOD << __FUNCTION__ << ": "
           << reply_.Context().IssuedNumbers({}) << " numbers issued."
           << std::endl;
}

UserCommandProcessor::UserCommandProcessor(
    Server& server,
    const opentxs::api::server::Manager& manager)
    : server_(server)
    , manager_(manager)
{
}

bool UserCommandProcessor::add_numbers_to_nymbox(
    const TransactionNumber transactionNumber,
    const NumList& newNumbers,
    bool& savedNymbox,
    Ledger& nymbox,
    Identifier& nymboxHash) const
{
    if (false == nymbox.LoadNymbox()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Error loading nymbox."
              << std::endl;

        return false;
    }

    bool success = true;
    success &= nymbox.VerifyContractID();

    if (success) { success &= nymbox.VerifySignature(server_.GetServerNym()); }

    if (false == success) {
        otErr << OT_METHOD << __FUNCTION__ << ": Error veryfying nymbox."
              << std::endl;

        return false;
    }

    // Note: I decided against adding newly-requested transaction
    // numbers to existing OTTransaction::blanks in the Nymbox.
    // Why not? Because once the user downloads the Box Receipt, he will
    // think he has it already, when the Server meanwhile
    // has a new version of that same Box Receipt. But the user will
    // never re-download it if he believes that he already has
    // it.
    // Since the transaction can contain 10, 20, or 50 transaction
    // numbers now, we don't NEED to be able to combine them
    // anyway, since the problem is still effectively solved.

    std::unique_ptr<OTTransaction> transaction{nullptr};
    transaction.reset(OTTransaction::GenerateTransaction(
        server_.API().Wallet(),
        nymbox,
        OTTransaction::blank,
        originType::not_applicable,
        transactionNumber));

    if (transaction) {
        transaction->AddNumbersToTransaction(newNumbers);
        transaction->SignContract(server_.GetServerNym());
        transaction->SaveContract();
        // Any inbox/nymbox/outbox ledger will only itself contain
        // abbreviated versions of the receipts, including their hashes.
        //
        // The rest is stored separately, in the box receipt, which is
        // created whenever a receipt is added to a box, and deleted after a
        // receipt is removed from a box.
        transaction->SaveBoxReceipt(nymbox);

        nymbox.AddTransaction(*transaction.release());
        nymbox.ReleaseSignatures();
        nymbox.SignContract(server_.GetServerNym());
        nymbox.SaveContract();
        savedNymbox = nymbox.SaveNymbox(nymboxHash);
    } else {
        nymbox.CalculateNymboxHash(nymboxHash);
    }

    return success;
}

// ACKNOWLEDGMENTS OF REPLIES ALREADY RECEIVED (FOR OPTIMIZATION.)

// On the client side, whenever the client is DEFINITELY made aware of the
// existence of a server reply, he adds its request number to this list,
// which is sent along with all client-side requests to the server. The
// server reads the list on the incoming client message (and it uses these
// same functions to store its own internal list.) If the # already appears
// on its internal list, then it does nothing. Otherwise, it loads up the
// Nymbox and removes the replyNotice, and then adds the # to its internal
// list. For any numbers on the internal list but NOT on the client's list,
// the server removes from the internal list. (The client removed them when
// it saw the server's internal list, which the server sends with its
// replies.)
//
// This entire protocol, densely described, is unnecessary for OT to
// function, but is great for optimization, as it enables OT to avoid
// downloading all Box Receipts containing replyNotices, as long as the
// original reply was properly received when the request was originally sent
// (which is MOST of the time...) Thus we can eliminate most replyNotice
// downloads, and likely a large % of box receipt downloads as well.
void UserCommandProcessor::check_acknowledgements(ReplyMessage& reply) const
{
    auto& context = reply.Context();
    // The server reads the list of acknowledged replies from the incoming
    // client message... If we add any acknowledged replies to the server-side
    // list, we will want to save (at the end.)
    auto numlist_ack_reply = reply.Acknowledged();
    const auto nymID = Identifier::Factory(context.RemoteNym().ID());
    Ledger nymbox(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        nymID,
        nymID,
        context.Server());

    if (nymbox.LoadNymbox() && nymbox.VerifySignature(server_.GetServerNym())) {
        bool bIsDirtyNymbox = false;

        for (auto& it : numlist_ack_reply) {
            const std::int64_t lRequestNum = it;
            // If the # already appears on its internal list, then it does
            // nothing. (It must have already done
            // whatever it needed to do, since it already has the number
            // recorded as acknowledged.)
            //
            // Otherwise, if the # does NOT already appear on server's
            // internal list, then it loads up the
            // Nymbox and removes the replyNotice, and then adds the # to
            // its internal list for safe-keeping.
            if (!context.VerifyAcknowledgedNumber(lRequestNum)) {
                // Verify whether a replyNotice exists in the Nymbox, with
                // that lRequestNum
                OTTransaction* pReplyNotice =
                    nymbox.GetReplyNotice(lRequestNum);

                if (nullptr != pReplyNotice) {
                    // If so, remove it...
                    const bool bDeleted =
                        pReplyNotice->DeleteBoxReceipt(nymbox);
                    const bool bRemoved = nymbox.RemoveTransaction(
                        pReplyNotice->GetTransactionNum());  // deletes
                    pReplyNotice = nullptr;
                    // (pReplyNotice is deleted, below this point,
                    // automatically by the above Remove call.)

                    if (!bDeleted || !bRemoved)
                        Log::Error(
                            "UserCommandProcessor::ProcessUserCommand: "
                            "Failed trying "
                            "to delete a box receipt, or "
                            "while removing its stub from the Nymbox.\n");

                    if (bRemoved) { bIsDirtyNymbox = true; }
                }

                context.AddAcknowledgedNumber(lRequestNum);
            }
        }

        if (bIsDirtyNymbox) {
            nymbox.ReleaseSignatures();
            nymbox.SignContract(server_.GetServerNym());
            nymbox.SaveContract();
            nymbox.SaveNymbox(Identifier::Factory());
        }
    }

    context.FinishAcknowledgements(numlist_ack_reply);
    reply.SetAcknowledgments(context);
}

bool UserCommandProcessor::check_client_isnt_server(
    const Identifier& nymID,
    const Nym& serverNym)
{
    const auto& serverNymID = serverNym.ID();
    const bool bNymIsServerNym = serverNymID == nymID;

    if (bNymIsServerNym) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Server nym is not allowed to act as a client." << std::endl;

        return false;
    }

    return true;
}

bool UserCommandProcessor::check_client_nym(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    const auto& nym = reply.Context().RemoteNym();

    if (false == msgIn.VerifySignature(nym)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to verify message signature." << std::endl;

        return false;
    }

    otInfo << OT_METHOD << __FUNCTION__
           << ": Message signature verification successful." << std::endl;

    return true;
}

bool UserCommandProcessor::check_message_notary(
    const Identifier& notaryID,
    const Identifier& realNotaryID)
{
    // Validate the server ID, to keep users from intercepting a valid requst
    // and sending it to the wrong server.
    if (false == (realNotaryID == notaryID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid server ID ("
              << String(notaryID) << ") sent in command request." << std::endl;

        return false;
    }

    otLog4 << OT_METHOD << __FUNCTION__
           << ": Received valid Notary ID with command request." << std::endl;

    return true;
}

bool UserCommandProcessor::check_ping_notary(const Message& msgIn) const
{
    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_check_notary_id);

    proto::AsymmetricKeyType keytypeAuthent =
        static_cast<proto::AsymmetricKeyType>(msgIn.keytypeAuthent_);

    auto nymAuthentKey = crypto::key::Asymmetric::Factory(
        keytypeAuthent, msgIn.m_strNymPublicKey);

    if (false == bool(nymAuthentKey.get())) { return false; }

    // Not all contracts are signed with the authentication key, but messages
    // are.
    if (!msgIn.VerifyWithKey(nymAuthentKey)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Signature verification failed!"
              << std::endl;

        return false;
    }

    otLog3 << OT_METHOD << __FUNCTION__ << ": Signature verified! The message "
           << "WAS signed by the Private Authentication Key inside the message."
           << std::endl;

    return true;
}

bool UserCommandProcessor::check_request_number(
    const Message& msgIn,
    const RequestNumber& correctNumber) const
{
    const RequestNumber messageNumber = msgIn.m_strRequestNum.ToLong();

    if (correctNumber != messageNumber) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Request number sent in this message (" << messageNumber
              << ") does not match the one in the context (" << correctNumber
              << ")" << std::endl;

        return false;
    }

    otLog3 << OT_METHOD << __FUNCTION__ << "Request number in this message "
           << messageNumber << " does match the one in the context."
           << std::endl;

    return true;
}

bool UserCommandProcessor::check_server_lock(const Identifier& nymID)
{
    if (false == ServerSettings::__admin_server_locked) { return true; }

    if (isAdmin(nymID)) { return true; }

    otErr << OT_METHOD << __FUNCTION__ << ": Nym " << String(nymID)
          << " failed attempt to message the server, while server is in "
          << "**LOCK DOWN MODE**" << std::endl;

    return false;
}

bool UserCommandProcessor::check_usage_credits(ReplyMessage& reply) const
{
    const auto nymfile = reply.Context().Nymfile("");

    OT_ASSERT(nymfile);

    const bool creditsRequired = ServerSettings::__admin_usage_credits;
    const bool needsCredits = nymfile->GetUsageCredits() >= 0;
    const bool checkCredits =
        creditsRequired && needsCredits && (false == isAdmin(nymfile->ID()));

    if (checkCredits) {
        auto nymFile = reply.Context().mutable_Nymfile("");
        const auto& credits = nymFile.It().GetUsageCredits();

        if (0 == credits) {
            otErr << OT_METHOD << __FUNCTION__ << ": Nym "
                  << nymFile.It().ID().str() << " Out of usage credits."
                  << std::endl;

            return false;
        }

        nymFile.It().SetUsageCredits(credits - 1);
    }

    return true;
}

bool UserCommandProcessor::cmd_add_claim(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_request_admin);

    const auto& context = reply.Context();
    const auto& nymID = context.RemoteNym().ID();
    const auto requestingNym = String(nymID);
    const std::uint32_t section = msgIn.m_strNymID2.ToUint();
    const std::uint32_t type = msgIn.m_strInstrumentDefinitionID.ToUint();
    const std::string value = msgIn.m_strAcctID.Get();
    const bool primary = msgIn.m_bBool;
    std::set<std::uint32_t> attributes;

    if (primary) { attributes.insert(proto::CITEMATTR_PRIMARY); }

    attributes.insert(proto::CITEMATTR_ACTIVE);

    Claim claim{"", section, type, value, 0, 0, attributes};

    String overrideNym;
    bool keyExists = false;
    server_.API().Config().Check_str(
        "permissions", "override_nym_id", overrideNym, keyExists);
    const bool haveAdmin = keyExists && overrideNym.Exists();
    const bool isAdmin = haveAdmin && (overrideNym == requestingNym);

    if (isAdmin) {
        auto nym =
            server_.API().Wallet().mutable_Nym(server_.GetServerNym().ID());
        otErr << "Before:\n" << std::string(nym.Claims()) << std::endl;
        nym.AddClaim(claim);
        otErr << "After:\n" << std::string(nym.Claims()) << std::endl;
    }

    return true;
}

bool UserCommandProcessor::cmd_check_nym(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    const auto& targetNym = msgIn.m_strNymID2;
    reply.SetTargetNym(targetNym);

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_check_nym);

    auto nym = server_.API().Wallet().Nym(Identifier::Factory(targetNym));

    if (nym) {
        reply.SetPayload(proto::ProtoAsData(nym->asPublicNym()));
        reply.SetSuccess(true);
    }

    return true;
}

// If the client wants to delete an asset account, the server will allow it...
// ...IF: the Inbox and Outbox are both EMPTY. AND the Balance must be empty as
// well!
bool UserCommandProcessor::cmd_delete_asset_account(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    reply.SetAccount(msgIn.m_strAcctID);

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_del_asset_acct);

    const auto accountID = Identifier::Factory(msgIn.m_strAcctID);
    const auto& context = reply.Context();
    const auto& serverNym = *context.Nym();
    auto account = server_.API().Wallet().mutable_Account(accountID);

    if (false == bool(account)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Error loading account "
              << String(accountID) << std::endl;

        return false;
    }

    const auto balance = account.get().GetBalance();

    if (balance != 0) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to delate account "
              << String(accountID) << " with non-zero balance " << balance
              << std::endl;

        return false;
    }

    std::unique_ptr<Ledger> inbox(account.get().LoadInbox(serverNym));
    std::unique_ptr<Ledger> outbox(account.get().LoadOutbox(serverNym));

    if (false == bool(inbox)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error loading inbox for account " << String(accountID)
              << std::endl;

        return false;
    }

    if (false == bool(outbox)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error loading outbox for account " << String(accountID)
              << std::endl;

        return false;
    }

    const auto inboxTransactions = inbox->GetTransactionCount();
    const auto outboxTransactions = outbox->GetTransactionCount();

    if (inboxTransactions > 0) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to delete account "
              << String(accountID) << " with " << inboxTransactions
              << " open inbox transactions." << std::endl;

        return false;
    }

    if (outboxTransactions > 0) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to delete account "
              << String(accountID) << " with " << inboxTransactions
              << " open outbox transactions." << std::endl;

        return false;
    }

    const auto& contractID = account.get().GetInstrumentDefinitionID();
    auto contract = server_.API().Wallet().UnitDefinition(contractID);

    if (false == bool(contract)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load unit definition"
              << String(contractID) << std::endl;

        return false;
    }

    if (contract->Type() == proto::UNITTYPE_SECURITY) {
        if (false == contract->EraseAccountRecord(
                         server_.API().DataFolder(), accountID)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to delete account record " << String(contractID)
                  << std::endl;

            return false;
        }
    }

    reply.SetSuccess(true);
    auto nymfile = server_.API().Wallet().mutable_Nymfile(
        reply.Context().RemoteNym().ID(), __FUNCTION__);
    auto& theAccountSet = nymfile.It().GetSetAssetAccounts();
    theAccountSet.erase(String(accountID).Get());
    account.Release();
    server_.API().Wallet().DeleteAccount(accountID);
    reply.DropToNymbox(false);

    return true;
}

// If a user requests to delete his own Nym, the server will allow it.
// IF: If the transaction numbers are all closable (available on both lists).
// AND if the Nymbox is empty. AND if there are no cron items open, AND if
// there are no asset accounts! (Delete them / Close them all FIRST! Or this
// fails.)
bool UserCommandProcessor::cmd_delete_user(ReplyMessage& reply) const
{
    auto nymfile = server_.API().Wallet().mutable_Nymfile(
        reply.Context().RemoteNym().ID(), __FUNCTION__);
    const auto& msgIn = reply.Original();
    auto& context = reply.Context();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_del_user_acct);

    const auto& nymID = context.RemoteNym().ID();
    const auto& server = context.Server();
    const auto& serverNym = *context.Nym();
    auto nymbox = load_nymbox(nymID, server, serverNym, true);

    if (false == bool(nymbox)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to load or verify nymbox." << std::endl;

        return false;
    }

    if (nymbox->GetTransactionCount() > 0) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed due to nymbox not empty." << std::endl;

        return false;
    }

    if (0 < context.OpenCronItems()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed due to open cron items."
              << std::endl;

        return false;
    }

    if (nymfile.It().GetSetAssetAccounts().size() > 0) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed due to open asset accounts." << std::endl;

        return false;
    }

    if (context.hasOpenTransactions()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed due to open transactions." << std::endl;

        return false;
    }

    reply.SetSuccess(true);

    // The Nym may have some numbers signed out, but none of them have come
    // through and been "used but not closed" yet. (That is, removed from
    // transaction num list but still on issued num list.) If they had (i.e.
    // if the previous elseif just above had discovered mismatched counts)
    // then we wouldn't be able to delete the Nym until those transactions
    // were closed. Since we know the counts match perfectly, here we remove
    // all the numbers. The client side must know to remove all the numbers
    // as well, when it receives a successful reply that the nym was
    // "deleted."
    context.Reset();
    // TODO: We may also need to mark the Nymbox, as well as the credential
    // files, as "Marked For Deletion."

    reply.DropToNymbox(false);

    return true;
}

bool UserCommandProcessor::cmd_get_account_data(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    reply.SetAccount(msgIn.m_strAcctID);

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_inbox);
    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_outbox);
    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_acct);

    const auto& context = reply.Context();
    const auto& nymID = context.RemoteNym().ID();
    const auto& serverID = context.Server();
    const auto& serverNym = *context.Nym();
    const auto accountID = Identifier::Factory(msgIn.m_strAcctID);
    auto account = server_.API().Wallet().mutable_Account(accountID);

    if (false == bool(account)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load account "
              << String(accountID) << std::endl;

        return false;
    }

    OT_ASSERT(account.get().GetNymID() == nymID);

    const auto inbox = load_inbox(nymID, accountID, serverID, serverNym, false);

    if (false == bool(inbox)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load or verify inbox for account "
              << String(accountID) << std::endl;

        return false;
    }

    const auto outbox =
        load_outbox(nymID, accountID, serverID, serverNym, false);

    if (false == bool(outbox)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load or verify outbox for account "
              << String(accountID) << std::endl;

        return false;
    }

    auto inboxHash = Identifier::Factory();
    auto outboxHash = Identifier::Factory();
    String serializedAccount{};
    String serializedInbox{};
    String serializedOutbox{};
    account.get().SaveContractRaw(serializedAccount);
    inbox->SaveContractRaw(serializedInbox);
    inbox->CalculateInboxHash(inboxHash);
    outbox->SaveContractRaw(serializedOutbox);
    outbox->CalculateOutboxHash(outboxHash);
    reply.SetPayload(serializedAccount);
    reply.SetPayload2(serializedInbox);
    reply.SetPayload3(serializedOutbox);
    reply.SetInboxHash(inboxHash);
    reply.SetOutboxHash(outboxHash);
    reply.SetSuccess(true);

    return true;
}

// the "accountID" on this message will contain the NymID if retrieving a
// boxreceipt for the Nymbox. Otherwise it will contain an AcctID if retrieving
// a boxreceipt for an Asset Acct.
bool UserCommandProcessor::cmd_get_box_receipt(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    const auto boxType = msgIn.m_lDepth;
    const TransactionNumber number = msgIn.m_lTransactionNum;
    reply.SetAccount(msgIn.m_strAcctID);
    reply.SetDepth(boxType);
    reply.SetTransactionNumber(number);

    switch (boxType) {
        case NYMBOX_DEPTH: {
            OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_nymbox)
        } break;
        case INBOX_DEPTH: {
            OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_inbox)
        } break;
        case OUTBOX_DEPTH: {
            OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_outbox)
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid box type."
                  << std::endl;

            return false;
        }
    }

    const auto& context = reply.Context();
    const auto& nymID = context.RemoteNym().ID();
    const auto& serverID = context.Server();
    const auto& serverNym = *context.Nym();
    const auto accountID = Identifier::Factory(msgIn.m_strAcctID);
    std::unique_ptr<Ledger> box{};

    switch (boxType) {
        case NYMBOX_DEPTH: {
            box = load_nymbox(nymID, serverID, serverNym, false);
        } break;
        case INBOX_DEPTH: {
            box = load_inbox(nymID, accountID, serverID, serverNym, false);
        } break;
        case OUTBOX_DEPTH: {
            box = load_outbox(nymID, accountID, serverID, serverNym, false);
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid box type."
                  << std::endl;

            return false;
        }
    }

    if (false == bool(box)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load or verify box."
              << std::endl;

        return false;
    }

    auto transaction = box->GetTransaction(number);

    if (nullptr == transaction) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Transaction not found: " << number << std::endl;

        return false;
    }

    box->LoadBoxReceipt(number);
    // The above call will replace transaction, inside box, with the full
    // version (instead of the abbreviated version) of that transaction, meaning
    // that the transaction pointer is now a bad pointer, if that call was
    // successful. Therefore we just call GetTransaction() AGAIN. This way,
    // whether LoadBoxReceipt() failed or not (perhaps it's legacy data and is
    // already not abbreviated, and thus the LoadBoxReceipt call failed, but
    // that's doesn't mean we're going to fail HERE, now does it?)
    transaction = box->GetTransaction(number);

    if (false == verify_transaction(transaction, serverNym)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid box item."
              << std::endl;

        return false;
    }

    reply.SetSuccess(true);
    reply.SetPayload(String(*transaction));

    return true;
}

bool UserCommandProcessor::cmd_get_instrument_definition(
    ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    reply.SetInstrumentDefinitionID(msgIn.m_strInstrumentDefinitionID);

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_contract);

    const auto contractID =
        Identifier::Factory(msgIn.m_strInstrumentDefinitionID);

    auto serialized = Data::Factory();
    auto unitDefiniton = server_.API().Wallet().UnitDefinition(contractID);
    // Perhaps the provided ID is actually a server contract, not an
    // instrument definition?
    auto server = server_.API().Wallet().Server(contractID);

    if (unitDefiniton) {
        reply.SetSuccess(true);
        serialized = proto::ProtoAsData(unitDefiniton->PublicContract());
        reply.SetPayload(serialized);
    } else if (server) {
        reply.SetSuccess(true);
        serialized = proto::ProtoAsData(server->PublicContract());
        reply.SetPayload(serialized);
    }

    return true;
}

// Get the list of markets on this server.
bool UserCommandProcessor::cmd_get_market_list(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_market_list);

    Armored output{};
    std::int32_t count{0};
    reply.SetSuccess(server_.Cron().GetMarketList(output, count));

    if (reply.Success()) {
        reply.SetDepth(count);

        if (0 < count) {
            reply.ClearRequest();
            reply.SetPayload(output);
        }
    }

    return true;
}

// Get the publicly-available list of offers on a specific market.
bool UserCommandProcessor::cmd_get_market_offers(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    reply.SetTargetNym(msgIn.m_strNymID2);

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_market_offers);

    auto depth = msgIn.m_lDepth;

    if (depth < 0) { depth = 0; }

    auto market =
        server_.Cron().GetMarket(Identifier::Factory(msgIn.m_strNymID2));

    if (nullptr == market) { return false; }

    Armored output{};
    std::int32_t nOfferCount{0};
    reply.SetSuccess(market->GetOfferList(output, depth, nOfferCount));

    if (reply.Success()) {
        reply.SetDepth(nOfferCount);

        if (0 < nOfferCount) {
            reply.ClearRequest();
            reply.SetPayload(output);
        }
    }

    return true;
}

// Get a report of recent trades that have occurred on a specific market.
bool UserCommandProcessor::cmd_get_market_recent_trades(
    ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    reply.SetTargetNym(msgIn.m_strNymID2);

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_market_recent_trades);

    auto market =
        server_.Cron().GetMarket(Identifier::Factory(msgIn.m_strNymID2));

    if (nullptr == market) { return false; }

    Armored output;
    std::int32_t count = 0;
    reply.SetSuccess(market->GetRecentTradeList(output, count));

    if (reply.Success()) {
        reply.SetDepth(count);

        if (0 < count) {
            reply.ClearRequest();
            reply.SetPayload(output);
        }
    }

    return true;
}

#if OT_CASH
bool UserCommandProcessor::cmd_get_mint(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    reply.SetInstrumentDefinitionID(msgIn.m_strInstrumentDefinitionID);

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_mint);

    const auto& unitID = msgIn.m_strInstrumentDefinitionID;
    auto mint = manager_.GetPublicMint(Identifier::Factory(unitID));

    if (mint) {
        reply.SetSuccess(true);
        reply.SetPayload(String(*mint));
    }

    return true;
}
#endif  // OT_CASH

// Get the offers that a specific Nym has placed on a specific market.
bool UserCommandProcessor::cmd_get_nym_market_offers(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_nym_market_offers);

    const auto& nymID = reply.Context().RemoteNym().ID();

    Armored output{};
    std::int32_t count{0};
    reply.SetSuccess(server_.Cron().GetNym_OfferList(output, nymID, count));

    if (reply.Success()) {
        reply.SetDepth(count);

        if (0 < count) {
            reply.ClearRequest();
            reply.SetPayload(output);
        }
    }

    return true;
}

bool UserCommandProcessor::cmd_get_nymbox(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_nymbox);

    auto& context = reply.Context();
    const auto& nymID = context.RemoteNym().ID();
    const auto& serverID = context.Server();
    const auto& serverNym = *context.Nym();
    const auto originalNymboxHash = context.LocalNymboxHash();
    auto newNymboxHash = Identifier::Factory();
    bool bSavedNymbox{false};
    auto nymbox = load_nymbox(nymID, serverID, serverNym, false);

    if (false == bool(nymbox)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load nymbox."
              << std::endl;
        reply.SetNymboxHash(originalNymboxHash);

        return false;
    }

    reply.SetSuccess(true);
    reply.SetPayload(String(*nymbox));

    if (bSavedNymbox) {
        context.SetLocalNymboxHash(newNymboxHash);
    } else {
        nymbox->CalculateNymboxHash(newNymboxHash);
        context.SetLocalNymboxHash(newNymboxHash);
    }

    reply.SetNymboxHash(newNymboxHash);

    return true;
}

// This command is special because it's the only one that doesn't require a
// request number. All of the other commands, below, will fail above if the
// proper request number isn't included in the message.  They will already have
// failed by this point if they // didn't verify.
bool UserCommandProcessor::cmd_get_request_number(ReplyMessage& reply) const
{
    auto& context = reply.Context();
    auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_requestnumber);

    const auto& serverNym = *context.Nym();
    auto number = context.Request();

    if (0 == number) {
        number = 1;
        context.SetRequest(number);
    }

    reply.SetRequestNumber(number);
    const auto NOTARY_ID = Identifier::Factory(server_.GetServerID());
    auto EXISTING_NYMBOX_HASH = context.LocalNymboxHash();

    if (String(EXISTING_NYMBOX_HASH).Exists()) {
        reply.SetNymboxHash(EXISTING_NYMBOX_HASH);
    } else {
        const auto& nymID = context.RemoteNym().ID();
        auto nymbox = load_nymbox(nymID, NOTARY_ID, serverNym, false);

        if (nymbox) {
            nymbox->CalculateNymboxHash(EXISTING_NYMBOX_HASH);
            context.SetLocalNymboxHash(EXISTING_NYMBOX_HASH);
            reply.SetNymboxHash(EXISTING_NYMBOX_HASH);
        }
    }

    reply.SetSuccess(true);

    return true;
}

bool UserCommandProcessor::cmd_get_transaction_numbers(
    ReplyMessage& reply) const
{
    auto& context = reply.Context();
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_trans_nums);

    // A few client requests, and a few server replies (not exactly matched up)
    // will include a copy of the NymboxHash.  The server may reject certain
    // client requests that have a bad value here (since it would be out of sync
    // anyway); the client is able to see the server's hash and realize to
    // re-download the nymbox and other intermediary files.
    const auto nCount = context.AvailableNumbers();
    const bool hashMatch = context.NymboxHashMatch();
    const auto& nymID = context.RemoteNym().ID();

    if (context.HaveLocalNymboxHash()) {
        reply.SetNymboxHash(context.LocalNymboxHash());
    }

    if (!hashMatch) {
        otErr << OT_METHOD << __FUNCTION__ << ": Rejecting message since "
              << "nymbox hash doesn't match." << std::endl;

        return false;
    }

    if (nCount > MAX_UNUSED_NUMBERS) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << String(nymID)
              << " already has more than 50 unused transaction numbers."
              << std::endl;

        return false;
    }

    auto NYMBOX_HASH = Identifier::Factory();
    bool bSuccess = true;
    bool bSavedNymbox = false;
    const auto& serverID = context.Server();
    Ledger theLedger(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        nymID,
        nymID,
        serverID);
    NumList theNumlist;

    for (std::int32_t i = 0; i < ISSUE_NUMBER_BATCH; i++) {
        TransactionNumber number{0};

        if (!server_.GetTransactor().issueNextTransactionNumber(number)) {
            number = 0;
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error issuing next transaction number." << std::endl;
            bSuccess = false;
            break;
        }

        theNumlist.Add(number);
    }

    TransactionNumber transactionNumber{0};

    if (bSuccess) {
        const bool issued = server_.GetTransactor().issueNextTransactionNumber(
            transactionNumber);

        if (false == issued) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error issuing transaction number." << std::endl;
            bSuccess = false;
        }
    }

    if (bSuccess) {
        reply.SetSuccess(add_numbers_to_nymbox(
            transactionNumber,
            theNumlist,
            bSavedNymbox,
            theLedger,
            NYMBOX_HASH));
    }

    if (bSavedNymbox) {
        context.SetLocalNymboxHash(NYMBOX_HASH);
    } else if (reply.Success()) {
        theLedger.CalculateNymboxHash(NYMBOX_HASH);
        context.SetLocalNymboxHash(NYMBOX_HASH);
    }

    reply.SetNymboxHash(NYMBOX_HASH);

    return true;
}

/// An existing user is creating an issuer account (that he will not control)
/// based on a basket of currencies.
bool UserCommandProcessor::cmd_issue_basket(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_issue_basket);

    auto serialized = proto::DataToProto<proto::UnitDefinition>(
        Data::Factory(msgIn.m_ascPayload));

    if (false == proto::Validate(serialized, VERBOSE)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid contract."
              << std::endl;

        return false;
    }

    if (proto::UNITTYPE_BASKET != serialized.type()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid contract type."
              << std::endl;

        return false;
    }

    // The basket ID should be the same on all servers.
    // The basket contract ID will be unique on each server.
    //
    // The contract ID of the basket is calculated based on the UNSIGNED portion
    // of the contract (so it is unique on every server) and for the same reason
    // with the AccountID removed before calculating.
    auto basketAccountID = Identifier::Factory();
    const auto BASKET_ID = BasketContract::CalculateBasketID(serialized);

    const bool exists = server_.GetTransactor().lookupBasketAccountID(
        BASKET_ID, basketAccountID);

    if (exists) {
        otErr << OT_METHOD << __FUNCTION__ << ": Basket already exists."
              << std::endl;

        return false;
    }

    // Let's make sure that all the sub-currencies for this basket are available
    // on this server. NOTE: this also prevents someone from using another
    // basket as a sub-currency UNLESS it already exists on this server. (For
    // example, they couldn't use a basket contract from some other server,
    // since it wouldn't be issued here...) Also note that
    // registerInstrumentDefinition explicitly prevents baskets from being
    // issued -- you HAVE to use issueBasket for creating any basket currency.
    // Taken in tandem, this insures that the only possible way to have a basket
    // currency as a sub-currency is if it's already issued on this server.
    for (auto& it : serialized.basket().item()) {
        const auto& subcontractID = it.unit();
        auto contract = server_.API().Wallet().UnitDefinition(
            Identifier::Factory(subcontractID));

        if (!contract) {
            otErr << OT_METHOD << __FUNCTION__ << ": Missing subcurrency "
                  << subcontractID << std::endl;

            return false;
        }
    }

    const auto& context = reply.Context();
    const auto& serverID = context.Server();
    const auto& serverNym = context.Nym();
    const auto& serverNymID = context.Nym()->ID();

    // We need to actually create all the sub-accounts. This loop also sets the
    // Account ID onto the basket items (which formerly was blank, from the
    // client.) This loop also adds the BASKET_ID and the NEW ACCOUNT ID to a
    // map on the server for later reference.
    for (auto& it : *serialized.mutable_basket()->mutable_item()) {
        auto newAccount = server_.API().Wallet().CreateAccount(
            serverNymID,
            serverID,
            Identifier::Factory(it.unit()),
            *serverNym,
            Account::basketsub,
            0);

        if (newAccount) {
            String newAccountID;
            newAccount.get().GetIdentifier(newAccountID);
            it.set_account(newAccountID.Get());
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to create subaccount." << std::endl;

            return false;
        }
    }

    if (false == BasketContract::FinalizeTemplate(
                     server_.API().Wallet(), serverNym, serialized)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to finalize basket contract." << std::endl;

        return false;
    }

    if (proto::UNITTYPE_BASKET != serialized.type()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid basket contract."
              << std::endl;

        return false;
    }

    const auto contract = server_.API().Wallet().UnitDefinition(serialized);

    if (false == bool(contract)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to construct basket contract object." << std::endl;

        return false;
    }

    const auto contractID = contract->ID();
    reply.SetInstrumentDefinitionID(String(contractID));

    // I don't save this here. Instead, I wait for AddBasketAccountID and then I
    // call SaveMainFile after that. See below.

    // TODO need better code for reverting when something screws up halfway
    // through a change. If I add this contract, it's not enough to just "not
    // save" the file. I actually need to re-load the file in order to TRULY
    // "make sure" this won't screw something up on down the line.

    // Once the new Asset Type is generated, from which the BasketID can be
    // requested at will, next we need to create the issuer account for that new
    // Asset Type.  (We have the instrument definition ID and the contract file.
    // Now let's create the issuer account the same as we would for any normal
    // issuer account.)
    //
    // The issuer account is special compared to a normal issuer account,
    // because within its walls, it must store an OTAccount for EACH
    // sub-contract, in order to store the reserves. That's what makes the
    // basket work.

    auto basketAccount = server_.API().Wallet().CreateAccount(
        serverNymID, serverID, contractID, *serverNym, Account::basket, 0);

    if (false == bool(basketAccount)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to instantiate basket account." << std::endl;

        return false;
    }

    basketAccount.get().GetIdentifier(basketAccountID);
    reply.SetSuccess(true);
    reply.SetAccount(String(basketAccountID));

    // So the server can later use the BASKET_ID (which is universal) to lookup
    // the account ID on this server corresponding to that basket. (The account
    // ID will be different from server to server, thus the need to be able to
    // look it up via the basket ID.)
    server_.GetTransactor().addBasketAccountID(
        BASKET_ID, basketAccountID, contractID);
    server_.GetMainFile().SaveMainFile();
#if OT_CASH
    manager_.UpdateMint(basketAccountID);
#endif  // OT_CASH
    basketAccount.Release();

    return true;
}

bool UserCommandProcessor::cmd_notarize_transaction(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    reply.SetAccount(msgIn.m_strAcctID);

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_notarize_transaction);

    auto& context = reply.Context();
    const auto& nymID = context.RemoteNym().ID();
    const auto& serverID = context.Server();
    const auto& serverNym = *context.Nym();
    const auto& serverNymID = serverNym.ID();
    const auto accountID = Identifier::Factory(msgIn.m_strAcctID);
    auto nymboxHash = Identifier::Factory();
    std::unique_ptr<Ledger> input(new Ledger(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        nymID,
        accountID,
        serverID));
    std::unique_ptr<Ledger> responseLedger(Ledger::GenerateLedger(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        serverNymID,
        accountID,
        serverID,
        Ledger::message,
        false));

    OT_ASSERT(input);
    OT_ASSERT(responseLedger);

    if (false == hash_check(context, nymboxHash)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nymbox hash mismatch."
              << std::endl;

        return false;
    }

    if (false == input->LoadLedgerFromString(String(msgIn.m_ascPayload))) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load input ledger."
              << std::endl;

        return false;
    }

    // Returning before this point will result in the reply message
    // m_bSuccess = false, and no reply ledger
    FinalizeResponse response(
        server_.API().Wallet(), serverNym, reply, *responseLedger);
    reply.SetSuccess(true);
    reply.DropToNymbox(true);
    // Returning after this point will result in the reply message
    // m_bSuccess = true, and a signed reply ledger containing at least one
    // transaction

    for (const auto& it : input->GetTransactionMap()) {
        const auto transaction = it.second;

        if (nullptr == transaction) {
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid input ledger."
                  << std::endl;

            OT_FAIL;
        }

        const auto inputNumber = transaction->GetTransactionNum();
        response.SetResponse(OTTransaction::GenerateTransaction(
            server_.API().Wallet(),
            *responseLedger,
            OTTransaction::error_state,
            originType::not_applicable,
            inputNumber));

        bool success{false};
        server_.GetNotary().NotarizeTransaction(
            context, *transaction, *response.Response(), success);

        if (response.Response()->IsCancelled()) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Success canceling transaction " << inputNumber
                  << " for nym " << String(nymID) << std::endl;
        } else {
            if (success) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Success processing transaction " << inputNumber
                      << " for nym " << String(nymID) << std::endl;
            } else {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failure processing transaction " << inputNumber
                      << " for nym " << String(nymID) << std::endl;
            }
        }

        OT_ASSERT_MSG(
            inputNumber == response.Response()->GetTransactionNum(),
            "Transaction number and response number should "
            "always be the same. (But this time, they weren't.)");

        if (false == responseLedger->AddTransaction(*response.Release())) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to add response transaction to response ledger."
                  << std::endl;

            OT_FAIL;
        }
    }

    return true;
}

bool UserCommandProcessor::cmd_ping_notary(ReplyMessage& reply) const
{
    reply.SetSuccess(check_ping_notary(reply.Original()));

    return true;
}

bool UserCommandProcessor::cmd_process_inbox(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    reply.SetAccount(msgIn.m_strAcctID);

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_process_inbox);

    auto& context = reply.Context();
    const auto& clientNym = context.RemoteNym();
    const auto& nymID = clientNym.ID();
    const auto& serverID = context.Server();
    const auto& serverNym = *context.Nym();
    const auto& serverNymID = serverNym.ID();
    const auto& nym = reply.Context().RemoteNym();
    const auto accountID = Identifier::Factory(msgIn.m_strAcctID);
    auto nymboxHash = Identifier::Factory();
    std::unique_ptr<Ledger> input(new Ledger(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        nymID,
        accountID,
        serverID));
    std::unique_ptr<Ledger> responseLedger(Ledger::GenerateLedger(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        serverNymID,
        accountID,
        serverID,
        Ledger::message,
        false));

    OT_ASSERT(input);
    OT_ASSERT(responseLedger);

    if (false == hash_check(context, nymboxHash)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nymbox hash mismatch."
              << std::endl;

        return false;
    }

    if (false == input->LoadLedgerFromString(String(msgIn.m_ascPayload))) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load input ledger."
              << std::endl;

        return false;
    }

    auto account = server_.API().Wallet().mutable_Account(accountID);

    if (false == bool(account)) { return false; }

    auto processInbox = input->GetTransaction(OTTransaction::processInbox);

    if (nullptr == processInbox) {
        otErr << OT_METHOD << __FUNCTION__
              << ": processInbox transaction not found in input ledger."
              << std::endl;

        return false;
    }

    const auto inputNumber = processInbox->GetTransactionNum();

    if (false == context.VerifyIssuedNumber(inputNumber)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Transaction number "
              << inputNumber << " is not issued to " << String(nymID)
              << std::endl;

        return false;
    }

    // The items' acct and server ID were already checked in VerifyContractID()
    // when they were loaded. Now this checks a little deeper, to verify
    // ownership, signatures, and transaction number on each item. That way
    // those things don't have to be checked for security over and over again in
    // the subsequent calls.
    if (false == processInbox->VerifyItems(nym)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to verify transaction items. " << std::endl;

        return false;
    }

    // We don't want any transaction number being used twice. (The number, at
    // this point, is STILL issued to the user, who is still responsible for
    // that number and must continue signing for it. All this means here is that
    // the user no longer has the number on his AVAILABLE list. Removal from
    // issued list happens separately.)
    if (false == context.ConsumeAvailable(inputNumber)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error removing available number " << inputNumber
              << std::endl;

        return false;
    }

    // Returning after this point will result in the reply message
    // m_bSuccess = true, and a signed reply ledger containing at least one
    // transaction
    FinalizeResponse response(
        server_.API().Wallet(), serverNym, reply, *responseLedger);
    reply.SetSuccess(true);
    reply.DropToNymbox(true);
    response.SetResponse(OTTransaction::GenerateTransaction(
        server_.API().Wallet(),
        *responseLedger,
        OTTransaction::error_state,
        originType::not_applicable,
        inputNumber));
    bool transactionSuccess{false};
    server_.GetNotary().NotarizeProcessInbox(
        context,
        account,
        *processInbox,
        *response.Response(),
        transactionSuccess);
    const auto consumed = context.ConsumeIssued(inputNumber);

    if (false == consumed) {
        otErr << OT_METHOD << __FUNCTION__ << ": Error removing issued number "
              << inputNumber << std::endl;

        OT_FAIL;
    }

    if (transactionSuccess) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Success processing process inbox " << inputNumber
              << " for nym " << String(nymID) << std::endl;
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure processing process inbox " << inputNumber
              << " for nym " << String(nymID) << std::endl;
    }

    OT_ASSERT_MSG(
        inputNumber == response.Response()->GetTransactionNum(),
        "Transaction number and response number should "
        "always be the same. (But this time, they weren't.)");

    if (false == responseLedger->AddTransaction(*response.Release())) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to add response transaction to response ledger."
              << std::endl;

        OT_FAIL;
    }

    return true;
}

bool UserCommandProcessor::cmd_process_nymbox(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_process_nymbox);

    auto& context = reply.Context();
    const auto& nymID = context.RemoteNym().ID();
    const auto& serverID = context.Server();
    const auto& serverNym = *context.Nym();
    const auto& serverNymID = serverNym.ID();
    auto nymboxHash = Identifier::Factory();
    std::unique_ptr<Ledger> input(new Ledger(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        nymID,
        nymID,
        serverID));
    std::unique_ptr<Ledger> responseLedger(Ledger::GenerateLedger(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        serverNymID,
        nymID,
        serverID,
        Ledger::message,
        false));

    OT_ASSERT(input);
    OT_ASSERT(responseLedger);

    if (false == hash_check(context, nymboxHash)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nymbox hash mismatch."
              << std::endl;

        return false;
    }

    if (false == input->LoadLedgerFromString(String(msgIn.m_ascPayload))) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load input ledger."
              << std::endl;

        return false;
    }

    // Returning before this point will result in the reply message
    // m_bSuccess = false, and no reply ledger
    FinalizeResponse response(
        server_.API().Wallet(), serverNym, reply, *responseLedger);
    reply.SetSuccess(true);
    reply.DropToNymbox(true);
    // Returning after this point will result in the reply message
    // m_bSuccess = true, and a signed reply ledger containing at least one
    // transaction

    for (const auto& it : input->GetTransactionMap()) {
        const auto transaction = it.second;

        if (nullptr == transaction) {
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid input ledger."
                  << std::endl;

            OT_FAIL;
        }

        const auto inputNumber = transaction->GetTransactionNum();
        response.SetResponse(OTTransaction::GenerateTransaction(
            server_.API().Wallet(),
            *responseLedger,
            OTTransaction::error_state,
            originType::not_applicable,
            transaction->GetTransactionNum()));

        bool success{false};
        server_.GetNotary().NotarizeProcessNymbox(
            context, *transaction, *response.Response(), success);

        if (success) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Success processing process nymbox " << inputNumber
                  << " for nym " << String(nymID) << std::endl;
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failure processing process nymbox " << inputNumber
                  << " for nym " << String(nymID) << std::endl;
        }

        OT_ASSERT_MSG(
            inputNumber == response.Response()->GetTransactionNum(),
            "Transaction number and response number should "
            "always be the same. (But this time, they weren't.)");

        if (false == responseLedger->AddTransaction(*response.Release())) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to add response transaction to response ledger."
                  << std::endl;

            OT_FAIL;
        }
    }

    return true;
}

bool UserCommandProcessor::cmd_query_instrument_definitions(
    ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_contract);

    std::unique_ptr<OTDB::Storable> pStorable(OTDB::DecodeObject(
        OTDB::STORED_OBJ_STRING_MAP, msgIn.m_ascPayload.Get()));
    auto inputMap = dynamic_cast<OTDB::StringMap*>(pStorable.get());

    if (nullptr == inputMap) { return false; }

    auto& map = inputMap->the_map;
    std::map<std::string, std::string> newMap{};

    for (auto& it : map) {
        const auto& unitID = it.first;
        const auto& status = it.second;

        if (unitID.empty()) { continue; }

        // TODO security: limit on length of this map? (sent through
        // user message...)
        // "exists" means, "Here's an instrument definition id. Please tell me
        // whether or not it's actually issued on this server." Future options
        // might include "issue", "audit", "contract", etc.
        if (0 == status.compare("exists")) {
            auto pContract = server_.API().Wallet().UnitDefinition(
                Identifier::Factory(unitID));

            if (pContract) {
                newMap[unitID] = "true";
            } else {
                newMap[unitID] = "false";
            }
        }
    }

    map.swap(newMap);
    const auto output = OTDB::EncodeObject(*inputMap);

    if (false == output.empty()) {
        reply.SetSuccess(true);
        reply.SetPayload(String(output));
    }

    return true;
}

/// An existing user is creating an asset account.
bool UserCommandProcessor::cmd_register_account(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_create_asset_acct);

    const auto& context = reply.Context();
    const auto& nymID = context.RemoteNym().ID();
    const auto& serverID = context.Server();
    const auto& serverNym = *context.Nym();
    const auto contractID =
        Identifier::Factory(msgIn.m_strInstrumentDefinitionID);
    auto account = server_.API().Wallet().CreateAccount(
        nymID, serverID, contractID, serverNym, Account::user, 0);

    // If we successfully create the account, then bundle it in the message XML
    // payload
    if (false == bool(account)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to create new account."
              << std::endl;

        return false;
    }

    auto contract = server_.API().Wallet().UnitDefinition(
        account.get().GetInstrumentDefinitionID());

    if (false == bool(contract)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load unit definition"
              << String(contractID) << std::endl;

        return false;
    }

    if (contract->Type() == proto::UNITTYPE_SECURITY) {
        // The instrument definition keeps a list of all accounts for that type.
        // (For shares, not for currencies.)
        if (false == contract->AddAccountRecord(
                         server_.API().DataFolder(), account.get())) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to add account record " << String(contractID)
                  << std::endl;

            return false;
        }
    }

    auto accountID = Identifier::Factory();
    account.get().GetIdentifier(accountID);
    Ledger outbox(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        nymID,
        accountID,
        serverID);
    Ledger inbox(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        nymID,
        accountID,
        serverID);
    bool inboxLoaded = inbox.LoadInbox();
    bool outboxLoaded = outbox.LoadOutbox();

    // ...or generate them otherwise...

    if (inboxLoaded) {
        inboxLoaded = inbox.VerifyAccount(serverNym);
    } else {
        inboxLoaded =
            inbox.CreateLedger(nymID, accountID, serverID, Ledger::inbox, true);

        if (inboxLoaded) { inboxLoaded = inbox.SignContract(serverNym); }

        if (inboxLoaded) { inboxLoaded = inbox.SaveContract(); }

        if (inboxLoaded) {
            inboxLoaded = account.get().SaveInbox(inbox, Identifier::Factory());
        }
    }

    if (true == outboxLoaded) {
        outboxLoaded = outbox.VerifyAccount(serverNym);
    } else {
        outboxLoaded = outbox.CreateLedger(
            nymID, accountID, serverID, Ledger::outbox, true);

        if (outboxLoaded) { outboxLoaded = outbox.SignContract(serverNym); }

        if (outboxLoaded) { outboxLoaded = outbox.SaveContract(); }

        if (outboxLoaded) {
            outboxLoaded =
                account.get().SaveOutbox(outbox, Identifier::Factory());
        }
    }

    if (false == inboxLoaded) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error generating inbox for account " << String(accountID)
              << std::endl;

        return false;
    }

    if (false == outboxLoaded) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error generating outbox for account " << String(accountID)
              << std::endl;

        return false;
    }

    reply.SetSuccess(true);
    reply.SetAccount(String(accountID));
    auto nymfile = server_.API().Wallet().mutable_Nymfile(
        reply.Context().RemoteNym().ID(), __FUNCTION__);
    auto& theAccountSet = nymfile.It().GetSetAssetAccounts();
    theAccountSet.insert(String(accountID).Get());
    reply.SetPayload(String(account.get()));
    reply.DropToNymbox(false);
    account.Release();

    return true;
}

bool UserCommandProcessor::cmd_register_contract(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_register_contract);

    const auto type = static_cast<ContractType>(msgIn.enum_);

    switch (type) {
        case (ContractType::NYM): {
            const auto nym = proto::DataToProto<proto::CredentialIndex>(
                Data::Factory(msgIn.m_ascPayload));
            reply.SetSuccess(bool(server_.API().Wallet().Nym(nym)));

            break;
        }
        case (ContractType::SERVER): {
            const auto server = proto::DataToProto<proto::ServerContract>(
                Data::Factory(msgIn.m_ascPayload));
            reply.SetSuccess(bool(server_.API().Wallet().Server(server)));

            break;
        }
        case (ContractType::UNIT): {
            const auto unit = proto::DataToProto<proto::UnitDefinition>(
                Data::Factory(msgIn.m_ascPayload));
            reply.SetSuccess(bool(server_.API().Wallet().UnitDefinition(unit)));

            break;
        }
        default: {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Invalid contract type: " << msgIn.enum_ << std::endl;
        }
    }

    return true;
}

/// An existing user is issuing a new currency.
bool UserCommandProcessor::cmd_register_instrument_definition(
    ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    reply.SetInstrumentDefinitionID(msgIn.m_strInstrumentDefinitionID);
    const auto contractID =
        Identifier::Factory(msgIn.m_strInstrumentDefinitionID);

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_issue_asset);

    auto contract = server_.API().Wallet().UnitDefinition(contractID);

    // Make sure the contract isn't already available on this server.
    if (contract) {
        otErr << OT_METHOD << __FUNCTION__ << ": Instrument definition "
              << String(contractID) << " already exists." << std::endl;

        return false;
    }

    const auto serialized = proto::DataToProto<proto::UnitDefinition>(
        Data::Factory(msgIn.m_ascPayload));

    if (proto::UNITTYPE_BASKET == serialized.type()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect unit type."
              << std::endl;

        return false;
    }

    contract = server_.API().Wallet().UnitDefinition(serialized);

    if (false == bool(contract)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid contract" << std::endl;

        return false;
    }

    if (contract->ID() != contractID) {
        otErr << OT_METHOD << __FUNCTION__ << ": ID mismatch" << std::endl;

        return false;
    }

    // Create an ISSUER account (like a normal account, except it can go
    // negative)
    const auto& context = reply.Context();
    const auto& nymID = context.RemoteNym().ID();
    const auto& serverID = context.Server();
    const auto& serverNym = *context.Nym();
    auto account = server_.API().Wallet().CreateAccount(
        nymID, serverID, contractID, serverNym, Account::issuer, 0);

    if (false == bool(account)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to generate issued account." << std::endl;

        return false;
    }

    reply.SetPayload(String(account.get()));
    auto accountID = Identifier::Factory();
    account.get().GetIdentifier(accountID);
    reply.SetAccount(String(accountID));
    server_.GetMainFile().SaveMainFile();

    if (false == account.get().InitBoxes(serverNym)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error initializing boxes for account " << accountID->str()
              << std::endl;

        return false;
    }

    reply.SetSuccess(true);
    account.Release();
    auto nymfile = server_.API().Wallet().mutable_Nymfile(
        reply.Context().RemoteNym().ID(), __FUNCTION__);
    auto& theAccountSet = nymfile.It().GetSetAssetAccounts();
    theAccountSet.insert(accountID->str());
    reply.DropToNymbox(false);
#if OT_CASH
    manager_.UpdateMint(contractID);
#endif  // OT_CASH

    return true;
}

bool UserCommandProcessor::cmd_register_nym(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_create_user_acct);

    auto serialized = proto::DataToProto<proto::CredentialIndex>(
        Data::Factory(reply.Original().m_ascPayload));
    auto sender_nym = server_.API().Wallet().Nym(serialized);

    if (false == bool(sender_nym)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Invalid nym: " << msgIn.m_strNymID << std::endl;

        return false;
    }

    otLog3 << OT_METHOD << __FUNCTION__ << ": Nym verified!" << std::endl;

    if (false == msgIn.VerifySignature(*sender_nym)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid signature."
              << sender_nym->ID().str() << std::endl;

        return false;
    }

    otLog3 << OT_METHOD << __FUNCTION__ << ": Signature verified!" << std::endl;

    if (false == reply.LoadContext()) { return false; }

    auto& context = reply.Context();

    // The below block is for the case where the Nym is re-registering, even
    // though he's already registered on this Notary.
    //
    // He ALREADY exists. We'll set success to true, and send him a copy of his
    // own nymfile.
    if (0 != context.Request()) { return reregister_nym(reply); }

    context.IncrementRequest();
    reply.SetSuccess(msgIn.WriteContract(
        OTFolders::UserAcct().Get(), msgIn.m_strNymID.Get()));

    if (false == reply.Success()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error saving new user account verification file."
              << std::endl;

        return true;
    }

    otErr << OT_METHOD << __FUNCTION__
          << ": Success saving new user account verification file."
          << std::endl;

    context.InitializeNymbox();
    context.SetRemoteNymboxHash(context.LocalNymboxHash());

    otErr << OT_METHOD << __FUNCTION__
          << ": Success registering Nym credentials." << std::endl;
    String strNymContents;
    // This will save the nymfile.
    auto nymfile =
        server_.API().Wallet().mutable_Nymfile(sender_nym->ID(), __FUNCTION__);
    nymfile.It().SerializeNymFile(strNymContents);
    reply.SetPayload(strNymContents);
    reply.SetSuccess(true);

    return true;
}

bool UserCommandProcessor::cmd_request_admin(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_request_admin);

    const String& requestingNym = msgIn.m_strNymID;
    const std::string candidate = requestingNym.Get();
    const std::string providedPassword = msgIn.m_strAcctID.Get();

    std::string overrideNym, password;
    bool notUsed = false;
    server_.API().Config().CheckSet_str(
        "permissions", "override_nym_id", "", overrideNym, notUsed);
    server_.API().Config().CheckSet_str(
        "permissions", "admin_password", "", password, notUsed);
    const bool noAdminYet = overrideNym.empty();
    const bool passwordSet = !password.empty();
    const bool readyForAdmin = (noAdminYet && passwordSet);
    const bool correctPassword = (providedPassword == password);
    const bool returningAdmin = (candidate == overrideNym);
    const bool duplicateRequest = (!noAdminYet && returningAdmin);

    if (false == correctPassword) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect password"
              << std::endl;

        return false;
    }

    if (readyForAdmin) {
        reply.SetSuccess(server_.API().Config().Set_str(
            "permissions", "override_nym_id", requestingNym, notUsed));

        if (reply.Success()) {
            otErr << __FUNCTION__ << ": override nym set." << std::endl;
            server_.API().Config().Save();
        }
    } else {
        if (duplicateRequest) {
            reply.SetSuccess(true);
        } else {
            otErr << __FUNCTION__ << ": Admin password empty or admin nym "
                  << "already set." << std::endl;
        }
    }

    return true;
}

bool UserCommandProcessor::cmd_send_nym_message(ReplyMessage& reply) const
{
    auto& context = reply.Context();
    const auto& sender = context.RemoteNym().ID();
    const auto& server = context.Server();
    const auto& msgIn = reply.Original();
    const auto& targetNym = msgIn.m_strNymID2;
    const auto recipient = Identifier::Factory(targetNym);
    reply.SetTargetNym(targetNym);

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_send_message);

    reply.SetSuccess(send_message_to_nym(server, sender, recipient, msgIn));

    if (false == reply.Success()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to send message to nym."
              << std::endl;
    }

    return true;
}

// msg, the request msg from payer, which is attached WHOLE to the Nymbox
// receipt. contains payment already.
// or pass pPayment instead: we will create our own msg here (with payment
// inside) to be attached to the receipt.
bool UserCommandProcessor::send_message_to_nym(
    const Identifier& NOTARY_ID,
    const Identifier& SENDER_NYM_ID,
    const Identifier& RECIPIENT_NYM_ID,
    const Message& pMsg) const
{
    return server_.DropMessageToNymbox(
        NOTARY_ID,
        SENDER_NYM_ID,
        RECIPIENT_NYM_ID,
        OTTransaction::message,
        pMsg);
}

bool UserCommandProcessor::cmd_trigger_clause(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_trigger_clause);

    const auto& number = msgIn.m_lTransactionNum;
    const auto& context = reply.Context();
    const auto& nym = context.RemoteNym();

    if (context.HaveLocalNymboxHash()) {
        const auto hash = context.LocalNymboxHash();
        reply.SetNymboxHash(hash);
    }

    if (false == context.NymboxHashMatch()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Rejecting message due to nymbox hash mismatch."
              << std::endl;

        return false;
    }

    OTSmartContract* smartContract{nullptr};
    auto cronItem = server_.Cron().GetItemByValidOpeningNum(number);

    if (nullptr == cronItem) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Can not find smart contract based on transaction number "
              << number << std::endl;

        return false;
    }

    smartContract = dynamic_cast<OTSmartContract*>(cronItem);

    if (nullptr == smartContract) {
        otErr << OT_METHOD << __FUNCTION__ << ": Cron item  " << number
              << " is not a smart contract." << std::endl;

        return false;
    }

    // FIND THE PARTY / PARTY NAME
    OTAgent* agent = nullptr;
    OTParty* party = smartContract->FindPartyBasedOnNymAsAgent(nym, &agent);

    if (nullptr == party) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to find party to this contract based on nym."
              << std::endl;

        return false;
    }

    const std::string clauseID = msgIn.m_strNymID2.Get();

    if (smartContract->CanExecuteClause(party->GetPartyName(), clauseID)) {
        // Execute the clause.
        mapOfClauses theMatchingClauses{};
        auto clause = smartContract->GetClause(clauseID);

        if (nullptr == clause) {
            otErr << OT_METHOD << __FUNCTION__ << ": Clause " << clauseID
                  << " does not exist." << std::endl;

            return false;
        }

        theMatchingClauses.insert({clauseID, clause});
        smartContract->ExecuteClauses(theMatchingClauses);

        if (smartContract->IsFlaggedForRemoval()) {
            otErr << OT_METHOD << __FUNCTION__ << ": Removing smart contract "
                  << smartContract->GetTransactionNum() << " from cron ."
                  << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Party "
              << party->GetPartyName() << " lacks permission to execute clause "
              << clauseID << std::endl;

        return false;
    }

    reply.SetSuccess(true);

    // If we just removed the smart contract from cron, that means a
    // finalReceipt was just dropped into the inboxes for the relevant asset
    // accounts. Once I process that receipt out of my inbox, (which will
    // require my processing out all related marketReceipts) then the closing
    // number will be removed from my list of responsibility.

    otErr << OT_METHOD << __FUNCTION__ << ": Party " << party->GetPartyName()
          << "successfully triggered clause: " << clauseID << std::endl;

    return true;
}

bool UserCommandProcessor::cmd_usage_credits(ReplyMessage& reply) const
{
    const auto& msgIn = reply.Original();
    reply.SetTargetNym(msgIn.m_strNymID2);
    reply.SetDepth(0);

    OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_usage_credits);

    const auto& adminContext = reply.Context();
    const auto& serverID = adminContext.Server();
    const auto& adminNym = adminContext.RemoteNym();
    const auto& adminNymID = adminNym.ID();
    const auto& serverNym = *adminContext.Nym();
    const bool admin = isAdmin(adminNymID);
    auto adjustment = msgIn.m_lDepth;

    if (false == admin) { adjustment = 0; }

    const auto targetNymID = Identifier::Factory(msgIn.m_strNymID2);
    OTIdentifier nymID = Identifier::Factory();

    if (targetNymID == adminNymID) {
        nymID = adminNymID;
    } else {
        if (false == admin) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": This command is only available to admin nym."
                  << std::endl;

            return false;
        }

        nymID = targetNymID;
    }

    auto nymfile =
        server_.API().Wallet().mutable_Nymfile(targetNymID, __FUNCTION__);
    auto nymbox = load_nymbox(targetNymID, serverID, serverNym, true);

    if (false == bool(nymbox)) {
        nymbox = create_nymbox(targetNymID, serverID, serverNym);
    }

    if (false == bool(nymbox)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load nymbox for "
              << targetNymID->str() << std::endl;

        return false;
    }

    const auto oldCredits = nymfile.It().GetUsageCredits();
    auto newCredits = oldCredits + adjustment;

    if (0 > newCredits) { newCredits = -1; }

    if (0 != adjustment) { nymfile.It().SetUsageCredits(newCredits); }
    reply.SetSuccess(true);

    if (ServerSettings::__admin_usage_credits) {
        reply.SetDepth(newCredits);
    } else {
        reply.SetDepth(-1);
    }

    return true;
}

std::unique_ptr<Ledger> UserCommandProcessor::create_nymbox(
    const Identifier& nymID,
    const Identifier& server,
    const Nym& serverNym) const
{
    std::unique_ptr<Ledger> nymbox(new Ledger(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        nymID,
        nymID,
        server));

    if (false == bool(nymbox)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to instantiate nymbox for " << String(nymID)
              << std::endl;

        return {};
    }

    if (false == nymbox->GenerateLedger(nymID, server, Ledger::nymbox, true)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to generate nymbox for "
              << String(nymID) << std::endl;

        return {};
    }

    auto notUsed = Identifier::Factory();

    if (false == save_nymbox(serverNym, notUsed, *nymbox)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to save nymbox for "
              << String(nymID) << std::endl;

        return {};
    }

    return nymbox;
}

// After EVERY / ANY transaction, plus certain messages, we drop a copy of the
// server's reply into the Nymbox.  This way we are GUARANTEED that the Nym will
// receive and process it. (And thus never get out of sync.)  This is the
// function used for doing that.
void UserCommandProcessor::drop_reply_notice_to_nymbox(
    const api::Wallet& wallet,
    const String& strMessage,
    const std::int64_t& lRequestNum,
    const bool bReplyTransSuccess,
    ClientContext& context,
    Server& server)
{
    const auto& nymID = context.RemoteNym().ID();
    const auto& serverID = context.Server();
    const auto& serverNym = *context.Nym();
    Ledger theNymbox(
        wallet, context.LegacyDataFolder(), nymID, nymID, serverID);
    bool bSuccessLoadingNymbox = theNymbox.LoadNymbox();

    if (true == bSuccessLoadingNymbox) {
        bSuccessLoadingNymbox =
            (theNymbox.VerifyContractID() &&
             theNymbox.VerifySignature(serverNym));
    }

    if (!bSuccessLoadingNymbox) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed loading or verifying Nymbox for user: "
              << String(nymID).Get() << std::endl;

        return;
    }

    TransactionNumber lReplyNoticeTransNum{0};
    const bool bGotNextTransNum =
        server.GetTransactor().issueNextTransactionNumber(lReplyNoticeTransNum);

    if (!bGotNextTransNum) {
        lReplyNoticeTransNum = 0;
        otErr << OT_METHOD << __FUNCTION__
              << "Error getting next transaction number for an "
              << "OTTransaction::replyNotice." << std::endl;

        return;
    }

    OTTransaction* pReplyNotice = OTTransaction::GenerateTransaction(
        wallet,
        theNymbox,
        OTTransaction::replyNotice,
        originType::not_applicable,
        lReplyNoticeTransNum);

    OT_ASSERT(nullptr != pReplyNotice);

    Item* pReplyNoticeItem = Item::CreateItemFromTransaction(
        *pReplyNotice, Item::replyNotice, Identifier::Factory());

    OT_ASSERT(nullptr != pReplyNoticeItem);

    // Nymbox notice is always a success. It's just a notice. (The message
    // inside it will have success/failure also, and any transaction inside that
    // will also.)
    pReplyNoticeItem->SetStatus(Item::acknowledgement);
    // Purpose of this notice is to carry a copy of server's reply message (to
    // certain requests, including all transactions.)
    pReplyNoticeItem->SetAttachment(strMessage);
    pReplyNoticeItem->SignContract(serverNym);
    pReplyNoticeItem->SaveContract();
    // the Transaction's destructor will cleanup the item. It "owns" it now. So
    // the client-side can quickly/easily match up the replyNotices in the
    // Nymbox with the request numbers of the messages that were sent. I think
    // this is actually WHY the server message low-level functions now RETURN
    // the request number. FYI: replyNotices will ONLY be in my Nymbox if the
    // MESSAGE was successful. (Meaning, the balance agreement might have
    // failed, and the transaction might have failed, but the MESSAGE ITSELF
    // must be a success, in order for the replyNotice to appear in the Nymbox.)
    pReplyNotice->AddItem(*pReplyNoticeItem);
    pReplyNotice->SetRequestNum(lRequestNum);
    pReplyNotice->SetReplyTransSuccess(bReplyTransSuccess);
    pReplyNotice->SignContract(serverNym);
    pReplyNotice->SaveContract();
    // Add the replyNotice to the nymbox. It takes ownership.
    theNymbox.AddTransaction(*pReplyNotice);
    theNymbox.ReleaseSignatures();
    theNymbox.SignContract(serverNym);
    theNymbox.SaveContract();
    auto NYMBOX_HASH = Identifier::Factory();
    theNymbox.SaveNymbox(NYMBOX_HASH);
    pReplyNotice->SaveBoxReceipt(theNymbox);
    context.SetLocalNymboxHash(NYMBOX_HASH);
}

bool UserCommandProcessor::hash_check(
    const ClientContext& context,
    Identifier& nymboxHash) const
{
    if (context.HaveLocalNymboxHash()) {
        nymboxHash.SetString(context.LocalNymboxHash()->str());
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Continuing without server side nymbox hash." << std::endl;

        return true;
    }

    if (false == context.HaveRemoteNymboxHash()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Continuing without client side nymbox hash." << std::endl;

        return true;
    }

    return context.NymboxHashMatch();
}

RequestNumber UserCommandProcessor::initialize_request_number(
    ClientContext& context) const
{
    auto requestNumber = context.Request();

    if (0 == requestNumber) {
        otErr << OT_METHOD << __FUNCTION__ << ": request number doesn't exist"
              << std::endl;

        requestNumber = context.IncrementRequest();

        OT_ASSERT(1 == requestNumber);
    }

    return requestNumber;
}

bool UserCommandProcessor::isAdmin(const Identifier& nymID)
{
    const auto adminNym = ServerSettings::GetOverrideNymID();

    if (adminNym.empty()) { return false; }

    return (0 == adminNym.compare(String(nymID).Get()));
}

std::unique_ptr<Ledger> UserCommandProcessor::load_inbox(
    const Identifier& nymID,
    const Identifier& accountID,
    const Identifier& serverID,
    const Nym& serverNym,
    const bool verifyAccount) const
{
    if (accountID == nymID) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid account ID "
              << String(accountID) << std::endl;

        return {};
    }

    std::unique_ptr<Ledger> inbox;
    inbox.reset(new Ledger(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        nymID,
        accountID,
        serverID));

    if (false == bool(inbox)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to instantiate inbox for " << String(nymID)
              << std::endl;

        return {};
    }

    if (false == inbox->LoadInbox()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load inbox for "
              << String(nymID) << std::endl;

        return {};
    }

    if (false == verify_box(nymID, *inbox, serverNym, verifyAccount)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to verify inbox for "
              << String(nymID) << std::endl;

        return {};
    }

    auto notUsed = Identifier::Factory();

    if (inbox->LoadedLegacyData()) { save_inbox(serverNym, notUsed, *inbox); }

    return inbox;
}

std::unique_ptr<Ledger> UserCommandProcessor::load_nymbox(
    const Identifier& nymID,
    const Identifier& serverID,
    const Nym& serverNym,
    const bool verifyAccount) const
{
    std::unique_ptr<Ledger> nymbox;
    nymbox.reset(new Ledger(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        nymID,
        nymID,
        serverID));

    if (false == bool(nymbox)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to instantiate nymbox for " << String(nymID)
              << std::endl;

        return {};
    }

    if (false == nymbox->LoadNymbox()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load nymbox for "
              << String(nymID) << std::endl;

        return {};
    }

    if (false == verify_box(nymID, *nymbox, serverNym, verifyAccount)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to verify nymbox for "
              << String(nymID) << std::endl;

        return {};
    }

    auto notUsed = Identifier::Factory();

    if (nymbox->LoadedLegacyData()) {
        save_nymbox(serverNym, notUsed, *nymbox);
    }

    return nymbox;
}

std::unique_ptr<Ledger> UserCommandProcessor::load_outbox(
    const Identifier& nymID,
    const Identifier& accountID,
    const Identifier& serverID,
    const Nym& serverNym,
    const bool verifyAccount) const
{
    if (accountID == nymID) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid account ID "
              << String(accountID) << std::endl;

        return {};
    }

    std::unique_ptr<Ledger> outbox;
    outbox.reset(new Ledger(
        server_.API().Wallet(),
        server_.API().DataFolder(),
        nymID,
        accountID,
        serverID));

    if (false == bool(outbox)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to instantiate outbox for " << String(nymID)
              << std::endl;

        return {};
    }

    if (false == outbox->LoadOutbox()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load outbox for "
              << String(nymID) << std::endl;

        return {};
    }

    if (false == verify_box(nymID, *outbox, serverNym, verifyAccount)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to verify outbox for "
              << String(nymID) << std::endl;

        return {};
    }

    auto notUsed = Identifier::Factory();

    if (outbox->LoadedLegacyData()) {
        save_outbox(serverNym, notUsed, *outbox);
    }

    return outbox;
}

bool UserCommandProcessor::ProcessUserCommand(
    const Message& msgIn,
    Message& msgOut)
{
    const std::string command(msgIn.m_strCommand.Get());
    const auto type = Message::Type(command);
    ReplyMessage reply(
        server_.API().Wallet(),
        server_.GetServerID(),
        server_.GetServerNym(),
        msgIn,
        server_,
        type,
        msgOut);

    if (false == reply.Init()) { return false; }

    otErr << "==> Received a " << command
          << " message. Nym: " << msgIn.m_strNymID << " ..." << std::endl;

    switch (type) {
        case MessageType::pingNotary: {
            return cmd_ping_notary(reply);
        }
        case MessageType::registerNym: {
            return cmd_register_nym(reply);
        }
        default: {
        }
    }

    if (false == reply.LoadContext()) { return false; }

    if (false == check_client_nym(reply)) { return false; }

    OT_ASSERT(reply.HaveContext());

    auto& context = reply.Context();
    context.SetRemoteNymboxHash(Identifier::Factory(msgIn.m_strNymboxHash));

    // ENTERING THE INNER SANCTUM OF SECURITY. If the user got all the way to
    // here, Then he has passed multiple levels of security, and all commands
    // below will assume the Nym is secure, validated, and loaded into memory
    // for use. But still need to verify the Request Number for all other
    // commands except Get Request Number itself... Request numbers start at 100
    // (currently). (Since certain special messages USE 1 already... Such as
    // messages that occur before requestnumbers are possible, like
    // RegisterNym.)
    auto requestNumber = initialize_request_number(context);

    // At this point, I now have the current request number for this nym in
    // requestNumber Let's compare it to the one that was sent in the message...
    // (This prevents attackers from repeat-sending intercepted messages to the
    // server.)
    if (MessageType::getRequestNumber != type) {
        if (false == check_request_number(msgIn, requestNumber)) {

            return false;
        }

        if (false == check_usage_credits(reply)) { return false; }

        context.IncrementRequest();
    }

    // At this point, we KNOW that it is EITHER a GetRequestNumber command,
    // which doesn't require a request number, OR it was some other command, but
    // the request number they sent in the command MATCHES the one that we just
    // read out of the file.

    // Therefore, we can process ALL messages below this point KNOWING that the
    // Nym is properly verified in all ways. No messages need to worry about
    // verifying the Nym, or about dealing with the Request Number. It's all
    // handled in here.
    check_acknowledgements(reply);

    switch (type) {
        case MessageType::getRequestNumber: {
            return cmd_get_request_number(reply);
        }
        case MessageType::getTransactionNumbers: {
            return cmd_get_transaction_numbers(reply);
        }
        case MessageType::checkNym: {
            return cmd_check_nym(reply);
        }
        case MessageType::sendNymMessage: {
            return cmd_send_nym_message(reply);
        }
        case MessageType::unregisterNym: {
            return cmd_delete_user(reply);
        }
        case MessageType::unregisterAccount: {
            return cmd_delete_asset_account(reply);
        }
        case MessageType::registerAccount: {
            return cmd_register_account(reply);
        }
        case MessageType::registerInstrumentDefinition: {
            return cmd_register_instrument_definition(reply);
        }
        case MessageType::issueBasket: {
            return cmd_issue_basket(reply);
        }
        case MessageType::notarizeTransaction: {
            return cmd_notarize_transaction(reply);
        }
        case MessageType::getNymbox: {
            return cmd_get_nymbox(reply);
        }
        case MessageType::getBoxReceipt: {
            return cmd_get_box_receipt(reply);
        }
        case MessageType::getAccountData: {
            return cmd_get_account_data(reply);
        }
        case MessageType::processNymbox: {
            return cmd_process_nymbox(reply);
        }
        case MessageType::processInbox: {
            return cmd_process_inbox(reply);
        }
        case MessageType::queryInstrumentDefinitions: {
            return cmd_query_instrument_definitions(reply);
        }
        case MessageType::getInstrumentDefinition: {
            return cmd_get_instrument_definition(reply);
        }
        case MessageType::getMint: {
#if OT_CASH
            return cmd_get_mint(reply);
#else
            return false;
#endif  // OT_CASH
        }
        case MessageType::getMarketList: {
            return cmd_get_market_list(reply);
        }
        case MessageType::getMarketOffers: {
            return cmd_get_market_offers(reply);
        }
        case MessageType::getMarketRecentTrades: {
            return cmd_get_market_recent_trades(reply);
        }
        case MessageType::getNymMarketOffers: {
            return cmd_get_nym_market_offers(reply);
        }
        case MessageType::triggerClause: {
            return cmd_trigger_clause(reply);
        }
        case MessageType::usageCredits: {
            return cmd_usage_credits(reply);
        }
        case MessageType::registerContract: {
            return cmd_register_contract(reply);
        }
        case MessageType::requestAdmin: {
            return cmd_request_admin(reply);
        }
        case MessageType::addClaim: {
            return cmd_add_claim(reply);
        }
        default: {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unknown command type: " << command << std::endl;

            reply.SetAccount(msgIn.m_strAcctID);
            String response(command);
            response.Concatenate("Response");
            reply.OverrideType(response);

            return false;
        }
    }
}

bool UserCommandProcessor::reregister_nym(ReplyMessage& reply) const
{
    auto& context = reply.Context();
    context.IncrementRequest();
    const auto& msgIn = reply.Original();
    otLog3 << OT_METHOD << __FUNCTION__
           << ": Re-registering nym: " << msgIn.m_strNymID << std::endl;

    String strNymContents;
    const auto& nym = reply.Context().RemoteNym();
    auto nymfile = reply.Context().Nymfile("");

    OT_ASSERT(nymfile)

    nymfile->SerializeNymFile(strNymContents);
    reply.SetPayload(strNymContents);
    const auto& serverNym = *context.Nym();
    const auto& serverID = context.Server();
    const auto& targetNymID = nym.ID();
    auto nymbox = load_nymbox(targetNymID, serverID, serverNym, false);

    if (false == bool(nymbox)) {
        context.InitializeNymbox();
        nymbox = load_nymbox(targetNymID, serverID, serverNym, false);
        context.SetRemoteNymboxHash(context.LocalNymboxHash());
    }

    if (false == bool(nymbox)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Error during nym "
              << "re-registration. Failed verifying or generating nymbox for "
              << "nym: " << msgIn.m_strNymID << std::endl;

        return false;
    }

    reply.SetSuccess(true);

    return true;
}

bool UserCommandProcessor::save_box(const Nym& nym, Ledger& box) const
{
    box.ReleaseSignatures();

    if (false == box.SignContract(nym)) { return false; }

    return box.SaveContract();
}

bool UserCommandProcessor::save_inbox(
    const Nym& nym,
    Identifier& hash,
    Ledger& inbox) const
{
    if (false == save_box(nym, inbox)) { return false; }

    if (false == inbox.SaveInbox(hash)) { return false; }

    return true;
}

bool UserCommandProcessor::save_nymbox(
    const Nym& nym,
    Identifier& hash,
    Ledger& nymbox) const
{
    if (false == save_box(nym, nymbox)) { return false; }

    if (false == nymbox.SaveNymbox(hash)) { return false; }

    return true;
}

bool UserCommandProcessor::save_outbox(
    const Nym& nym,
    Identifier& hash,
    Ledger& outbox) const
{
    if (false == save_box(nym, outbox)) { return false; }

    if (false == outbox.SaveOutbox(hash)) { return false; }

    return true;
}

bool UserCommandProcessor::verify_box(
    const Identifier& ownerID,
    Ledger& box,
    const Nym& nym,
    const bool full) const
{
    if (false == box.VerifyContractID()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to verify box ID for "
              << String(ownerID) << std::endl;

        return false;
    }

    if (full) {
        if (false == box.VerifyAccount(nym)) {
            otErr << OT_METHOD << __FUNCTION__ << ": Unable to verify box for "
                  << String(ownerID) << std::endl;

            return false;
        }
    } else {
        if (false == box.VerifySignature(nym)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to verify signature for " << String(ownerID)
                  << std::endl;

            return false;
        }
    }

    return true;
}

bool UserCommandProcessor::verify_transaction(
    const OTTransaction* transaction,
    const Nym& signer) const
{
    if (nullptr == transaction) { return false; }

    if (transaction->IsAbbreviated()) { return false; }

    if (false == transaction->VerifyContractID()) { return false; }

    return transaction->VerifySignature(signer);
}
}  // namespace opentxs::server
