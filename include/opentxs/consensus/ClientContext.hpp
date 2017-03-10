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

#include "opentxs/consensus/Context.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"

#include <set>

namespace opentxs
{

class Wallet;

class ClientContext : public Context
{
private:
    typedef Context ot_super;

    std::set<TransactionNumber> open_cron_items_;

    using ot_super::serialize;
    proto::Context serialize(const Lock& lock) const override;

    ClientContext() = delete;
    ClientContext(const ClientContext&) = delete;
    ClientContext(ClientContext&&) = delete;
    ClientContext& operator=(const ClientContext&) = delete;
    ClientContext& operator=(ClientContext&&) = delete;

public:
    ClientContext(
        const Identifier& local,
        const Identifier& remote,
        Wallet& wallet);
    ClientContext(const proto::Context& serialized, Wallet& wallet);

    proto::ConsensusType Type() const override;

    std::size_t OpenCronItems() const;
    bool VerifyCronItem(const TransactionNumber number) const;

    bool CloseCronItem(const TransactionNumber number);
    void FinishAcknowledgements(const std::set<RequestNumber>& req);
    bool OpenCronItem(const TransactionNumber number);


    ~ClientContext() = default;
};
} // namespace opentxs

#endif // OPENTXS_CONSENSUS_CLIENTCONTEXT_HPP
