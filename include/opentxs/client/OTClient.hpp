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

#include "opentxs/client/OTMessageBuffer.hpp"
#include "opentxs/client/OTMessageOutbuffer.hpp"
#include "opentxs/client/OTServerConnection.hpp"
#include "opentxs/core/Types.hpp"

#include <string>
#include <memory>

namespace opentxs
{

class Account;
class UnitDefinition;
class Ledger;
class ServerContract;
class OTWallet;

class OTClient
{
public:
    explicit OTClient(OTWallet* theWallet);

    bool connect(const std::string& endpoint,
                 const unsigned char* transportKey);

    inline OTMessageBuffer& GetMessageBuffer()
    {
        return m_MessageBuffer;
    }

    inline OTMessageOutbuffer& GetMessageOutbuffer()
    {
        return m_MessageOutbuffer;
    }

    void ProcessMessageOut(const ServerContract* pServerContract, Nym* pNym,
                           const Message& theMessage);
    bool ProcessInBuffer(const Message& theServerReply) const;

    EXPORT int32_t ProcessUserCommand(ClientCommandType requestedCommand,
                                      Message& theMessage, Nym& theNym,
                                      const ServerContract& theServer,
                                      const Account* pAccount = nullptr,
                                      int64_t lTransactionAmount = 0,
                                      const UnitDefinition* pMyUnitDefinition = nullptr,
                                      const Identifier* pHisNymID = nullptr,
                                      const Identifier* pHisAcctID = nullptr);

    bool processServerReply(std::shared_ptr<Message> theReply,
                            Ledger* pNymbox = nullptr);

    bool AcceptEntireNymbox(Ledger& theNymbox, const Identifier& theNotaryID,
                            const ServerContract& theServerContract,
                            Nym& theNym, Message& theMessage);

private:
    void ProcessIncomingTransactions(OTServerConnection& theConnection,
                                     const Message& theReply) const;
    void ProcessWithdrawalResponse(OTTransaction& theTransaction,
                                   const OTServerConnection& theConnection,
                                   const Message& theReply) const;
    void ProcessDepositResponse(OTTransaction& theTransaction,
                                const OTServerConnection& theConnection,
                                const Message& theReply) const;
    void ProcessPayDividendResponse(OTTransaction& theTransaction,
                                    const OTServerConnection& theConnection,
                                    const Message& theReply) const;

    void load_str_trans_add_to_ledger(const Identifier& the_nym_id,
                                      const String& str_trans,
                                      const String& str_box_type,
                                      const int64_t& lTransNum, Nym& the_nym,
                                      Ledger& ledger) const;

    struct ProcessServerReplyArgs;
    void setRecentHash(const Message& theReply, const String& strNotaryID,
                       Nym* pNym, bool setNymboxHash);
    bool processServerReplyTriggerClause(const Message& theReply,
                                         ProcessServerReplyArgs& args);
    bool processServerReplyGetRequestNumber(const Message& theReply,
                                            ProcessServerReplyArgs& args);
    bool processServerReplyCheckNym(const Message& theReply,
                                    ProcessServerReplyArgs& args);
    bool processServerReplyNotarizeTransaction(const Message& theReply,
                                               ProcessServerReplyArgs& args);
    bool processServerReplyGetTransactionNumbers(const Message& theReply,
                                                 ProcessServerReplyArgs& args);
    bool processServerReplyGetNymBox(const Message& theReply, Ledger* pNymbox,
                                     ProcessServerReplyArgs& args);
    bool processServerReplyGetBoxReceipt(const Message& theReply,
                                         Ledger* pNymbox,
                                         ProcessServerReplyArgs& args);
    bool processServerReplyProcessInbox(const Message& theReply,
                                        Ledger* pNymbox,
                                        ProcessServerReplyArgs& args);
    bool processServerReplyGetAccountData(const Message& theReply,
                                          Ledger* pNymbox,
                                          ProcessServerReplyArgs& args);
    bool processServerReplyGetInstrumentDefinition(
        const Message& theReply, ProcessServerReplyArgs& args);
    bool processServerReplyGetMint(const Message& theReply);
    bool processServerReplyGetMarketList(const Message& theReply);
    bool processServerReplyGetMarketOffers(const Message& theReply);
    bool processServerReplyGetMarketRecentTrades(const Message& theReply);
    bool processServerReplyGetNymMarketOffers(const Message& theReply);
    bool processServerReplyUnregisterNym(const Message& theReply,
                                         ProcessServerReplyArgs& args);
    bool processServerReplyUnregisterAccount(const Message& theReply,
                                             ProcessServerReplyArgs& args);
    bool processServerReplyRegisterInstrumentDefinition(
        const Message& theReply, ProcessServerReplyArgs& args);
    bool processServerReplyRegisterAccount(const Message& theReply,
                                           ProcessServerReplyArgs& args);

private:
    std::unique_ptr<OTServerConnection> m_pConnection;
    OTWallet* m_pWallet;
    OTMessageBuffer m_MessageBuffer;
    OTMessageOutbuffer m_MessageOutbuffer;
};

} // namespace opentxs

#endif // OPENTXS_CLIENT_OTCLIENT_HPP
