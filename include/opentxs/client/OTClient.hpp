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

#ifndef OPENTXS_CLIENT_OTCLIENT_HPP
#define OPENTXS_CLIENT_OTCLIENT_HPP

#include "opentxs/Version.hpp"

#include "opentxs/client/OTMessageBuffer.hpp"
#include "opentxs/client/OTMessageOutbuffer.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/Types.hpp"

#include <memory>
#include <string>

namespace opentxs
{
class Account;
class UnitDefinition;
class Ledger;
class Item;
class ServerContract;
class OTCronItem;
class OTWallet;
class PeerObject;

namespace api
{
class Activity;
class ContactManager;

namespace client
{
class Wallet;
}  // namespace client
}  // namespace api

class OTClient
{
public:
    explicit OTClient(
        OTWallet& theWallet,
        const api::Activity& activity,
        const api::ContactManager& contacts,
        const api::client::Wallet& wallet);

    inline OTMessageBuffer& GetMessageBuffer() { return m_MessageBuffer; }

    inline OTMessageOutbuffer& GetMessageOutbuffer()
    {
        return m_MessageOutbuffer;
    }

    void QueueOutgoingMessage(const Message& theMessage);

    int32_t ProcessUserCommand(
        const MessageType requestedCommand,
        ServerContext& context,
        Message& theMessage,
        const Account* pAccount = nullptr,
        const int64_t lTransactionAmount = 0,
        const UnitDefinition* pMyUnitDefinition = nullptr,
        const Identifier* pHisNymID = nullptr,
        const Identifier* pHisAcctID = nullptr);

    bool processServerReply(
        const std::set<ServerContext::ManagedNumber>& managed,
        ServerContext& context,
        std::shared_ptr<Message>& reply,
        Ledger* pNymbox = nullptr);

    bool AcceptEntireNymbox(
        Ledger& theNymbox,
        ServerContext& context,
        Message& theMessage);

private:
    OTWallet& m_pWallet;
    const api::Activity& activity_;
    const api::ContactManager& contacts_;
    const api::client::Wallet& wallet_;
    OTMessageBuffer m_MessageBuffer;
    OTMessageOutbuffer m_MessageOutbuffer;

    void ProcessIncomingTransaction(
        const Message& theReply,
        ServerContext& context,
        OTTransaction* pTransaction,
        String& strReceiptID) const;
    void ProcessIncomingTransactions(
        const Message& theReply,
        const Identifier& accountID,
        ServerContext& context) const;
#if OT_CASH
    void ProcessWithdrawalResponse(
        const Message& theReply,
        ServerContext& context,
        OTTransaction& theTransaction) const;
#endif  // OT_CASH
    void ProcessDepositResponse(
        const Message& theReply,
        const ServerContext& context,
        OTTransaction& theTransaction) const;
    void ProcessDepositChequeResponse(
        const ServerContext& context,
        Item* pReplyItem) const;
    void ProcessPayDividendResponse(OTTransaction& theTransaction) const;
    void load_str_trans_add_to_ledger(
        const Identifier& the_nym_id,
        const String& str_trans,
        const String& str_box_type,
        const TransactionNumber& lTransNum,
        const Nym& the_nym,
        Ledger& ledger) const;
    void setRecentHash(
        const Message& theReply,
        bool setNymboxHash,
        ServerContext& context);
    bool processServerReplyTriggerClause(
        const Message& theReply,
        ServerContext& context);
    bool processServerReplyCheckNym(
        const Message& theReply,
        ServerContext& context);
    bool processServerReplyNotarizeTransaction(
        const Message& theReply,
        const Identifier& accountID,
        ServerContext& context);
    bool processServerReplyGetTransactionNumbers(
        const Message& theReply,
        ServerContext& context);
    bool processServerReplyGetNymBox(
        const Message& theReply,
        Ledger* pNymbox,
        ServerContext& context);
    bool processServerReplyGetBoxReceipt(
        const Message& theReply,
        Ledger* pNymbox,
        ServerContext& context);
    bool processServerReplyProcessBox(
        const Message& theReply,
        const Identifier& accountID,
        Ledger* pNymbox,
        ServerContext& context);
    bool processServerReplyProcessInbox(
        const Message& theReply,
        const Identifier& accountID,
        Ledger* pNymbox,
        ServerContext& context,
        OTTransaction* pTransaction,
        OTTransaction* pReplyTransaction);
    bool processServerReplyProcessNymbox(
        const Message& theReply,
        Ledger* pNymbox,
        ServerContext& context,
        OTTransaction* pTransaction,
        OTTransaction* pReplyTransaction);
    bool processServerReplyGetAccountData(
        const Message& theReply,
        const Identifier& accountID,
        Ledger* pNymbox,
        ServerContext& context);
    bool processServerReplyGetInstrumentDefinition(
        const Message& theReply,
        ServerContext& context);
#if OT_CASH
    bool processServerReplyGetMint(const Message& theReply);
#endif  // OT_CASH
    bool processServerReplyGetMarketList(const Message& theReply);
    bool processServerReplyGetMarketOffers(const Message& theReply);
    bool processServerReplyGetMarketRecentTrades(const Message& theReply);
    bool processServerReplyGetNymMarketOffers(const Message& theReply);
    bool processServerReplyUnregisterNym(
        const Message& theReply,
        ServerContext& context);
    bool processServerReplyUnregisterAccount(
        const Message& theReply,
        ServerContext& context);
    bool processServerReplyRegisterInstrumentDefinition(
        const Message& theReply,
        const Identifier& accountID,
        ServerContext& context);
    bool processServerReplyRegisterAccount(
        const Message& theReply,
        const Identifier& accountID,
        ServerContext& context);
    bool createInstrumentNoticeFromPeerObject(
        const ServerContext& context,
        const std::unique_ptr<PeerObject>& peerObject,
        OTTransaction* pTxnPeerObject);
    void ProcessIncomingCronItemReply(
        Item* pReplyItem,
        std::unique_ptr<OTCronItem>& pCronItem,
        ServerContext& context,
        const TransactionNumber& lNymOpeningNumber,
        OTTransaction* pTransaction,
        const String& strCronItem) const;
};
}  // namespace opentxs

#endif  // OPENTXS_CLIENT_OTCLIENT_HPP
