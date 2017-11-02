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

#ifndef OPENTXS_SERVER_OTSERVER_HPP
#define OPENTXS_SERVER_OTSERVER_HPP

#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/network/ZMQ.hpp"
#include "opentxs/server/Transactor.hpp"
#include "opentxs/server/Notary.hpp"
#include "opentxs/server/MainFile.hpp"
#include "opentxs/server/UserCommandProcessor.hpp"

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <tuple>

namespace opentxs
{

class Identifier;
class Message;
class OTPayment;
class ServerContract;

class OTServer
{
    friend class Transactor;
    friend class MessageProcessor;
    friend class UserCommandProcessor;
    friend class MainFile;
    friend class PayDividendVisitor;
    friend class Notary;

public:
    EXPORT OTServer();

    EXPORT bool GetConnectInfo(std::string& hostname, std::uint32_t& port)
        const;
    EXPORT const Identifier& GetServerID() const;
    EXPORT const Nym& GetServerNym() const;
    EXPORT zcert_t* GetTransportKey() const;
    EXPORT bool IsFlaggedForShutdown() const;

    EXPORT void ActivateCron();
    EXPORT void Init(
        const std::map<std::string, std::string>& args,
        bool readOnly = false);
    EXPORT void ProcessCron();
    EXPORT std::int64_t computeTimeout() { return m_Cron.computeTimeout(); }

    EXPORT ~OTServer();

private:
    std::pair<std::string, std::string> parse_seed_backup(
        const std::string& input) const;

    void CreateMainFile(
        bool& mainFileExists,
        const std::map<std::string, std::string>& args);
    // Note: SendInstrumentToNym and SendMessageToNym CALL THIS.
    // They are higher-level, this is lower-level.
    bool DropMessageToNymbox(
        const Identifier& notaryID,
        const Identifier& senderNymID,
        const Identifier& recipientNymID,
        OTTransaction::transactionType transactionType,
        const Message* msg = nullptr,
        const String* messageString = nullptr,
        const char* command = nullptr);
    bool DropMessageToNymbox(
        const Identifier& notaryID,
        const Identifier& senderNymID,
        const Identifier& recipientNymID,
        OTTransaction::transactionType transactionType,
        const Message& msg);
    bool SendInstrumentToNym(
        const Identifier& notaryID,
        const Identifier& senderNymID,
        const Identifier& recipientNymID,
        const OTPayment* payment,
        const char* command);
    bool SendInstrumentToNym(
        const Identifier& notaryID,
        const Identifier& senderNymID,
        const Identifier& recipientNymID,
        const Message& msg);

private:
    const std::string DEFAULT_EXTERNAL_IP = "127.0.0.1";
    const std::string DEFAULT_BIND_IP = "127.0.0.1";
    const std::string DEFAULT_NAME = "localhost";
    const std::uint32_t DEFAULT_COMMAND_PORT = 7085;
    const std::uint32_t DEFAULT_NOTIFY_PORT = 7086;
    const std::uint32_t MIN_TCP_PORT = 1024;
    const std::uint32_t MAX_TCP_PORT = 63356;

    MainFile mainFile_;
    Notary notary_;
    Transactor transactor_;
    UserCommandProcessor userCommandProcessor_;

    String m_strWalletFilename;
    // Used at least for whether or not to write to the PID.
    bool m_bReadOnly{false};
    // If the server wants to be shut down, it can set
    // this flag so the caller knows to do so.
    bool m_bShutdownFlag{false};

    // A hash of the server contract
    Identifier m_strNotaryID;
    // A hash of the public key that signed the server contract
    String m_strServerNymID;
    // This is the server's own contract, containing its public key and
    // connect info.

    Nym m_nymServer;

    OTCron m_Cron;  // This is where re-occurring and expiring tasks go.
};

}  // namespace opentxs

#endif  // OPENTXS_SERVER_OTSERVER_HPP
