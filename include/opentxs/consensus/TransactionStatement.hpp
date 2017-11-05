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

#ifndef OPENTXS_CONSENSUS_TRANSACTIONSTATEMENT_HPP
#define OPENTXS_CONSENSUS_TRANSACTIONSTATEMENT_HPP

#include "opentxs/Version.hpp"

#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#include <set>
#include <string>

namespace opentxs
{

class TransactionStatement
{
private:
    std::string version_;
    std::string nym_id_;
    std::string notary_;
    std::set<TransactionNumber> available_;
    std::set<TransactionNumber> issued_;

    TransactionStatement() = delete;
    TransactionStatement(const TransactionStatement& rhs) = delete;
    TransactionStatement& operator=(const TransactionStatement& rhs) = delete;
    TransactionStatement& operator=(TransactionStatement&& rhs) = delete;

public:
    TransactionStatement(
        const std::string& notary,
        const std::set<TransactionNumber>& issued,
        const std::set<TransactionNumber>& available);
    TransactionStatement(const String& serialized);
    TransactionStatement(TransactionStatement&& rhs) = default;

    explicit operator String() const;

    const std::set<TransactionNumber>& Issued() const;
    const std::string& Notary() const;

    void Remove(const TransactionNumber& number);

    ~TransactionStatement() = default;
};
}  // namespace opentxs

#endif  // OPENTXS_CONSENSUS_TRANSACTIONSTATEMENT_HPP
