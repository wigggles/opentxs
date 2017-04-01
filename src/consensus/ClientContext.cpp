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

#include "opentxs/consensus/ClientContext.hpp"

namespace opentxs
{
ClientContext::ClientContext(const ConstNym& local, const ConstNym& remote)
      : ot_super(local, remote)
{
}

ClientContext::ClientContext(
    const proto::Context& serialized,
    const ConstNym& local,
    const ConstNym& remote)
    : ot_super(serialized, local, remote)
{
    if (serialized.has_clientcontext()) {
        for (const auto& it : serialized.clientcontext().opencronitems()) {
            open_cron_items_.insert(it);
        }
    }
}

void ClientContext::FinishAcknowledgements(const std::set<RequestNumber>& req)
{
    Lock lock(lock_);

    finish_acknowledgements(lock, req);
}

bool ClientContext::CloseCronItem(const TransactionNumber number)
{
    Lock lock(lock_);

    auto output = open_cron_items_.erase(number);

    lock.unlock();

    return (0 < output);
}

bool ClientContext::OpenCronItem(const TransactionNumber number)
{
    Lock lock(lock_);

    auto output = open_cron_items_.insert(number);

    lock.unlock();

    return output.second;
}

std::size_t ClientContext::OpenCronItems() const
{
    return open_cron_items_.size();
}

proto::Context ClientContext::serialize(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = serialize(lock, Type());
    auto& client = *output.mutable_clientcontext();
    client.set_version(output.version());

    for (const auto& it : open_cron_items_) {
        client.add_opencronitems(it);
    }

    return output;
}

proto::ConsensusType ClientContext::Type() const
{
    return proto::CONSENSUSTYPE_CLIENT;
}

bool ClientContext::VerifyCronItem(const TransactionNumber number) const
{
    return (0 < open_cron_items_.count(number));
}
} // namespace opentxs
