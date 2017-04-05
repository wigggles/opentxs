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
class TransactionStatement;
class Wallet;

class ServerContext : public Context
{
private:
    typedef Context ot_super;

    Identifier server_id_;
    std::atomic<TransactionNumber> highest_transaction_number_;
    std::set<TransactionNumber> tentative_transaction_numbers_;

    static void scan_number_set(
        const std::set<TransactionNumber>& input,
        TransactionNumber& highest,
        TransactionNumber& lowest);
    static void validate_number_set(
        const std::set<TransactionNumber>& input,
        const TransactionNumber limit,
        std::set<TransactionNumber>& good,
        std::set<TransactionNumber>& bad);

    std::unique_ptr<TransactionStatement> generate_statement(
        const Lock& lock,
        const std::set<TransactionNumber>& adding,
        const std::set<TransactionNumber>& without) const;
    using ot_super::serialize;
    proto::Context serialize(const Lock& lock) const override;

    bool remove_tentative_number(
        const Lock& lock,
        const TransactionNumber& number);
    TransactionNumber update_highest(
        const Lock& lock,
        const std::set<TransactionNumber>& numbers,
        std::set<TransactionNumber>& good,
        std::set<TransactionNumber>& bad);

    ServerContext() = delete;
    ServerContext(const ServerContext&) = delete;
    ServerContext(ServerContext&&) = delete;
    ServerContext& operator=(const ServerContext&) = delete;
    ServerContext& operator=(ServerContext&&) = delete;

public:
    ServerContext(
        const ConstNym& local,
        const ConstNym& remote,
        const Identifier& server);
    ServerContext(
        const proto::Context& serialized,
        const ConstNym& local,
        const ConstNym& remote);

    TransactionNumber Highest() const;
    const Identifier& Server() const;
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

    proto::ConsensusType Type() const override;

    ~ServerContext() = default;
};
} // namespace opentxs

#endif // OPENTXS_CONSENSUS_SERVERCONTEXT_HPP
