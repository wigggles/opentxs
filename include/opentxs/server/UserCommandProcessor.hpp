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

class ClientConnection;
class ClientContext;
class Identifier;
class OTServer;
class Message;
class Nym;
class String;

class UserCommandProcessor
{
public:
    UserCommandProcessor(OTServer* server);

    bool ProcessUserCommand(
        const Message& msgIn,
        Message& msgOut,
        ClientConnection* connection);

private:
    OTServer* server_{nullptr};

    void DropReplyNoticeToNymbox(
        const String& messageString,
        const std::int64_t& requestNum,
        const bool replyTransSuccess,
        ClientContext& context,
        Nym* actualNym = nullptr);
    bool SendMessageToNym(
        const Identifier& notaryID,
        const Identifier& senderNymID,
        const Identifier& recipientNymID,
        const Message& msg);
    void UserCmdAddClaim(Nym& nym, const Message& msgIn, Message& msgOut);
    void UserCmdCheckNym(Nym& nym, const Message& msgIn, Message& msgOut);
    void UserCmdDeleteAssetAcct(
        Nym& nym,
        ClientContext& context,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdDeleteUser(
        Nym& nym,
        ClientContext& context,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdGetAccountData(Nym& nym, const Message& msgIn, Message& msgOut);
    void UserCmdGetBoxReceipt(const Message& msgIn, Message& msgOut);
    void UserCmdGetInstrumentDefinition(const Message& msgIn, Message& msgOut);
    // Get the list of markets on this server.
    void UserCmdGetMarketList(Nym& nym, const Message& msgIn, Message& msgOut);
    // Get the publicly-available list of offers on a specific market.
    void UserCmdGetMarketOffers(
        Nym& nym,
        const Message& msgIn,
        Message& msgOut);
    // Get a report of recent trades that have occurred on a specific market.
    void UserCmdGetMarketRecentTrades(
        Nym& nym,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdGetMint(Nym& nym, const Message& msgIn, Message& msgOut);
    // Get the offers that a specific Nym has placed on a specific market.
    void UserCmdGetNymMarketOffers(
        Nym& nym,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdGetNymbox(
        Nym& nym,
        ClientContext& context,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdGetRequestNumber(
        Nym& nym,
        ClientContext& context,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdGetTransactionNumbers(
        Nym& nym,
        ClientContext& context,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdIssueBasket(Nym& nym, const Message& msgIn, Message& msgOut);
    void UserCmdNotarizeTransaction(
        Nym& nym,
        ClientContext& context,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdPingNotary(Nym& nym, const Message& msgIn, Message& msgOut);
    void UserCmdProcessInbox(
        Nym& nym,
        ClientContext& context,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdProcessNymbox(
        Nym& nym,
        ClientContext& context,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdQueryInstrumentDefinitions(
        Nym& nym,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdRegisterAccount(
        Nym& nym,
        ClientContext& context,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdRegisterContract(
        Nym& nym,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdRegisterInstrumentDefinition(
        Nym& nym,
        ClientContext& context,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdRequestAdmin(Nym& nym, const Message& msgIn, Message& msgOut);
    void UserCmdSendNymInstrument(
        Nym& nym,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdSendNymMessage(Nym& nym, const Message& msgIn, Message& msgOut);
    void UserCmdTriggerClause(
        Nym& nym,
        ClientContext& context,
        const Message& msgIn,
        Message& msgOut);
    void UserCmdUsageCredits(Nym& nym, const Message& msgIn, Message& msgOut);
};
}  // namespace opentxs

#endif  // OPENTXS_SERVER_USERCOMMANDPROCESSOR_HPP
