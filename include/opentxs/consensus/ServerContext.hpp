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

#ifndef OPENTXS_CONSENSUS_SERVERCONTEXT_HPP
#define OPENTXS_CONSENSUS_SERVERCONTEXT_HPP

#include "opentxs/consensus/Context.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"

#include <atomic>
#include <set>

namespace opentxs
{

class Item;
class OTTransaction;
class Message;
class ServerConnection;
class TransactionStatement;
class Wallet;

class ServerContext : public Context
{
public:
    ServerContext(
        const ConstNym& local,
        const ConstNym& remote,
        const Identifier& server,
        ServerConnection& connection);
    ServerContext(
        const proto::Context& serialized,
        const ConstNym& local,
        const ConstNym& remote,
        ServerConnection& connection);

    TransactionNumber Highest() const;
    std::unique_ptr<Item> Statement(const OTTransaction& owner) const;
    std::unique_ptr<Item> Statement(
        const OTTransaction& owner,
        const std::set<TransactionNumber>& adding) const;
    std::unique_ptr<TransactionStatement> Statement(
        const std::set<TransactionNumber>& adding,
        const std::set<TransactionNumber>& without) const;
    bool Verify(const TransactionStatement& statement) const;
    bool VerifyTentativeNumber(const TransactionNumber& number) const;

    bool AcceptIssuedNumber(const TransactionNumber& number);
    bool AcceptIssuedNumbers(const TransactionStatement& statement);
    bool AddTentativeNumber(const TransactionNumber& number);
    TransactionNumber NextTransactionNumber();
    bool RemoveTentativeNumber(const TransactionNumber& number);
    bool SetHighest(const TransactionNumber& highest);
    TransactionNumber UpdateHighest(
        const std::set<TransactionNumber>& numbers,
        std::set<TransactionNumber>& good,
        std::set<TransactionNumber>& bad);
    RequestNumber UpdateRequestNumber();
    RequestNumber UpdateRequestNumber(bool& sendStatus);

    proto::ConsensusType Type() const override;

    ~ServerContext() = default;

private:
    typedef Context ot_super;

    ServerConnection& connection_;

    std::mutex message_lock_{};
    std::atomic<TransactionNumber> highest_transaction_number_{0};
    std::set<TransactionNumber> tentative_transaction_numbers_{};

    static void scan_number_set(
        const std::set<TransactionNumber>& input,
        TransactionNumber& highest,
        TransactionNumber& lowest);
    static void validate_number_set(
        const std::set<TransactionNumber>& input,
        const TransactionNumber limit,
        std::set<TransactionNumber>& good,
        std::set<TransactionNumber>& bad);

    bool finalize_server_command(Message& command) const;
    std::unique_ptr<TransactionStatement> generate_statement(
        const Lock& lock,
        const std::set<TransactionNumber>& adding,
        const std::set<TransactionNumber>& without) const;
    std::unique_ptr<Message> initialize_server_command(
        const MessageType type) const;
    using ot_super::serialize;
    proto::Context serialize(const Lock& lock) const override;

    using ot_super::remove_acknowledged_number;
    bool remove_acknowledged_number(const Lock& lock, const Message& reply);
    bool remove_tentative_number(
        const Lock& lock,
        const TransactionNumber& number);
    TransactionNumber update_highest(
        const Lock& lock,
        const std::set<TransactionNumber>& numbers,
        std::set<TransactionNumber>& good,
        std::set<TransactionNumber>& bad);
    Identifier update_remote_hash(const Lock& lock, const Message& reply);

    ServerContext() = delete;
    ServerContext(const ServerContext&) = delete;
    ServerContext(ServerContext&&) = delete;
    ServerContext& operator=(const ServerContext&) = delete;
    ServerContext& operator=(ServerContext&&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CONSENSUS_SERVERCONTEXT_HPP
