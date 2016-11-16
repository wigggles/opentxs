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

#ifndef OPENTXS_SERVER_USERCOMMANDPROCESSOR_HPP
#define OPENTXS_SERVER_USERCOMMANDPROCESSOR_HPP

#include <cstdint>

namespace opentxs
{
class String;
class Message;
class Nym;
class OTServer;
class Identifier;
class ClientConnection;

class UserCommandProcessor
{
public:
    UserCommandProcessor(OTServer* server);

    bool ProcessUserCommand(Message& msgIn, Message& msgOut,
                            ClientConnection* connection, Nym* nym);

private:
    OTServer* server_{nullptr};

    bool SendMessageToNym(const Identifier& notaryID,
                          const Identifier& senderNymID,
                          const Identifier& recipientNymID,
                          Message* msg = nullptr,
                          const String* messageString = nullptr);

    void DropReplyNoticeToNymbox(const Identifier& notaryID,
                                 const Identifier& nymID,
                                 const String& messageString,
                                 const int64_t& requestNum,
                                 const bool replyTransSuccess,
                                 Nym* actualNym = nullptr);

    void UserCmdPingNotary(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdCheckNym(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdSendNymMessage(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdSendNymInstrument(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdGetRequestNumber(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdGetTransactionNumbers(Nym& nym, Message& msgIn,
                                      Message& msgOut);
    void UserCmdRegisterInstrumentDefinition(Nym& nym, Message& msgIn,
                                             Message& msgOut);
    void UserCmdIssueBasket(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdGetBoxReceipt(Message& msgIn, Message& msgOut);
    void UserCmdDeleteUser(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdDeleteAssetAcct(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdRegisterAccount(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdNotarizeTransaction(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdGetNymbox(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdGetAccountData(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdGetInstrumentDefinition(Message& msgIn, Message& msgOut);
    void UserCmdGetMint(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdProcessInbox(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdProcessNymbox(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdRegisterContract(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdUsageCredits(Nym& nym, Message& msgIn, Message& msgOut);
    void UserCmdTriggerClause(Nym& nym, Message& msgIn, Message& msgOut);

    void UserCmdQueryInstrumentDefinitions(Nym& nym, Message& msgIn,
                                           Message& msgOut);

    // Get the list of markets on this server.
    void UserCmdGetMarketList(Nym& nym, Message& msgIn, Message& msgOut);

    // Get the publicly-available list of offers on a specific market.
    void UserCmdGetMarketOffers(Nym& nym, Message& msgIn, Message& msgOut);

    // Get a report of recent trades that have occurred on a specific market.
    void UserCmdGetMarketRecentTrades(Nym& nym, Message& msgIn,
                                      Message& msgOut);

    // Get the offers that a specific Nym has placed on a specific market.
    void UserCmdGetNymMarketOffers(Nym& nym, Message& msgIn, Message& msgOut);

    void UserCmdRequestAdmin(Nym& nym, Message& msgIn, Message& msgOut);
};
} // namespace opentxs

#endif // OPENTXS_SERVER_USERCOMMANDPROCESSOR_HPP
