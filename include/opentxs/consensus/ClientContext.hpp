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

#ifndef OPENTXS_CONSENSUS_CLIENTCONTEXT_HPP
#define OPENTXS_CONSENSUS_CLIENTCONTEXT_HPP

#include "opentxs/Version.hpp"

#include "opentxs/consensus/Context.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"

#include <set>

namespace opentxs
{

class TransactionStatement;
class Wallet;

class ClientContext : public Context
{
private:
    typedef Context ot_super;

public:
    ClientContext(
        const ConstNym& local,
        const ConstNym& remote,
        const Identifier& server);
    ClientContext(
        const proto::Context& serialized,
        const ConstNym& local,
        const ConstNym& remote,
        const Identifier& server);

    bool hasOpenTransactions() const;
    std::size_t IssuedNumbers(const std::set<TransactionNumber>& exclude) const;
    std::size_t OpenCronItems() const;
    proto::ConsensusType Type() const override;
    bool Verify(
        const TransactionStatement& statement,
        const std::set<TransactionNumber>& excluded,
        const std::set<TransactionNumber>& included) const;
    bool VerifyCronItem(const TransactionNumber number) const;
    using ot_super::VerifyIssuedNumber;
    bool VerifyIssuedNumber(
        const TransactionNumber& number,
        const std::set<TransactionNumber>& exclude) const;

    bool AcceptIssuedNumbers(std::set<TransactionNumber>& newNumbers);
    bool CloseCronItem(const TransactionNumber number) override;
    void FinishAcknowledgements(const std::set<RequestNumber>& req);
    bool IssueNumber(const TransactionNumber& number);
    bool OpenCronItem(const TransactionNumber number) override;

    ~ClientContext() = default;

private:
    std::set<TransactionNumber> open_cron_items_{};

    using ot_super::serialize;
    proto::Context serialize(const Lock& lock) const override;

    ClientContext() = delete;
    ClientContext(const ClientContext&) = delete;
    ClientContext(ClientContext&&) = delete;
    ClientContext& operator=(const ClientContext&) = delete;
    ClientContext& operator=(ClientContext&&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CONSENSUS_CLIENTCONTEXT_HPP
