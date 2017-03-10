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

#include "opentxs/consensus/ServerContext.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
ServerContext::ServerContext(
    const Identifier& local,
    const Identifier& remote,
    const Identifier& server,
    Wallet& wallet)
      : ot_super(local, remote, wallet)
      , server_id_(server)
{
    highest_transaction_number_.store(0);
}

ServerContext::ServerContext(const proto::Context& serialized, Wallet& wallet)
    : ot_super(serialized, wallet)
    , server_id_()
{
    if (serialized.has_servercontext()) {
        auto& context = serialized.servercontext();
        server_id_ = Identifier(context.serverid());
        highest_transaction_number_.store(context.highesttransactionnumber());

        for (const auto& it : context.tentativerequestnumber()) {
            tentative_transaction_numbers_.insert(it);
        }
    }
}

bool ServerContext::AddTentativeNumber(const TransactionNumber& number)
{
    Lock lock(lock_);

    if (number < highest_transaction_number_.load()) { return false; }

    auto output = tentative_transaction_numbers_.insert(number);

    lock.unlock();

    return output.second;
}

TransactionNumber ServerContext::Highest() const
{
    return highest_transaction_number_.load();
}

bool ServerContext::RemoveTentativeNumber(const TransactionNumber& number)
{
    Lock lock(lock_);

    auto output = tentative_transaction_numbers_.erase(number);

    lock.unlock();

    return (0 < output);
}

void ServerContext::scan_number_set(
    const std::set<TransactionNumber>& input,
    TransactionNumber& highest,
    TransactionNumber& lowest)
{
    highest = 0;
    lowest = 0;

    if (0 < input.size()) {
        lowest = *input.cbegin();
        highest = *input.crbegin();
    }
}

proto::Context ServerContext::serialize(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = serialize(lock, Type());
    auto& server = *output.mutable_servercontext();
    server.set_version(output.version());
    server.set_serverid(String(server_id_).Get());
    server.set_highesttransactionnumber(highest_transaction_number_.load());

    for (const auto& it : tentative_transaction_numbers_) {
        server.add_tentativerequestnumber(it);
    }

    return output;
}

bool ServerContext::SetHighest(const TransactionNumber& highest)
{
    Lock lock(lock_);

    if (highest >= highest_transaction_number_.load()) {
        highest_transaction_number_.store(highest);

        return true;
    }

    return false;
}

proto::ConsensusType ServerContext::Type() const
{
    return proto::CONSENSUSTYPE_SERVER;
}

TransactionNumber ServerContext::UpdateHighest(
    const std::set<TransactionNumber>& numbers,
    std::set<TransactionNumber>& good,
    std::set<TransactionNumber>& bad)
{
    TransactionNumber output = 0;  // 0 is success.
    TransactionNumber highest = 0;
    TransactionNumber lowest = 0;

    scan_number_set(numbers, highest, lowest);

    Lock lock(lock_);
    const TransactionNumber oldHighest = highest_transaction_number_.load();

    validate_number_set(numbers, oldHighest, good, bad);

    if ((lowest > 0) && (lowest <= oldHighest)) {
        // Return the first invalid number
        output = lowest;
    }

    if (!good.empty()) {
        if (0 != oldHighest) {
            otOut << __FUNCTION__ << ": Raising Highest Transaction Number "
                  << "from " << oldHighest << " to " << highest << "."
                  << std::endl;
        } else {
            otOut << __FUNCTION__ << ": Creating Highest Transaction Number "
                  << "entry for this server as '" << highest << "'."
                  << std::endl;
        }

        highest_transaction_number_.store(highest);
    }

    return output;
}

void ServerContext::validate_number_set(
    const std::set<TransactionNumber>& input,
    const TransactionNumber limit,
    std::set<TransactionNumber>& good,
    std::set<TransactionNumber>& bad)
{
    for (const auto& it : input) {
        if (it <= limit) {
            otWarn << __FUNCTION__ << ": New transaction number is "
                   << "less-than-or-equal-to last known 'highest trans number' "
                   << "record. (Must be seeing the same server reply for a "
                   << "second time, due to a receipt in my Nymbox.) FYI, last "
                   << "known 'highest' number received: " << limit
                   << " (Current 'violator': " << it<< ") Skipping..."
                   << std::endl;
            bad.insert(it);
        } else {
            good.insert(it);
        }
    }
}

bool ServerContext::VerifyTentativeNumber(const TransactionNumber& number) const
{
    return (0 < tentative_transaction_numbers_.count(number));
}
} // namespace opentxs
