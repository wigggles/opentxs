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

#ifndef OPENTXS_API_CLIENT_SYNC_HPP
#define OPENTXS_API_CLIENT_SYNC_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Types.hpp"

#include <cstdint>
#include <tuple>

#define DEFAULT_PROCESS_INBOX_ITEMS 5

namespace opentxs
{
namespace api
{
namespace client
{
class Sync
{
public:
    EXPORT virtual bool AcceptIncoming(
        const Identifier& nymID,
        const Identifier& accountID,
        const Identifier& serverID,
        const std::size_t max = DEFAULT_PROCESS_INBOX_ITEMS) const = 0;
    EXPORT virtual Depositability CanDeposit(
        const Identifier& recipientNymID,
        const OTPayment& payment) const = 0;
    EXPORT virtual Depositability CanDeposit(
        const Identifier& recipientNymID,
        const Identifier& accountID,
        const OTPayment& payment) const = 0;
    EXPORT virtual Messagability CanMessage(
        const Identifier& senderNymID,
        const Identifier& recipientContactID) const = 0;
    EXPORT virtual OTIdentifier DepositPayment(
        const Identifier& recipientNymID,
        const std::shared_ptr<const OTPayment>& payment) const = 0;
    EXPORT virtual OTIdentifier DepositPayment(
        const Identifier& recipientNymID,
        const Identifier& accountID,
        const std::shared_ptr<const OTPayment>& payment) const = 0;
    EXPORT virtual OTIdentifier FindNym(const Identifier& nymID) const = 0;
    EXPORT virtual OTIdentifier FindNym(
        const Identifier& nymID,
        const Identifier& serverIDHint) const = 0;
    EXPORT virtual OTIdentifier FindServer(const Identifier& serverID) const = 0;
    EXPORT virtual const Identifier& IntroductionServer() const = 0;
    EXPORT virtual OTIdentifier MessageContact(
        const Identifier& senderNymID,
        const Identifier& contactID,
        const std::string& message) const = 0;
    EXPORT virtual std::pair<ThreadStatus, OTIdentifier> MessageStatus(
        const Identifier& taskID) const = 0;
    EXPORT virtual OTIdentifier PayContact(
        const Identifier& senderNymID,
        const Identifier& contactID,
        std::shared_ptr<const OTPayment>& payment) const = 0;
#if OT_CASH
    EXPORT virtual OTIdentifier PayContactCash(
        const Identifier& senderNymID,
        const Identifier& contactID,
        std::shared_ptr<const Purse>& recipientCopy,
        std::shared_ptr<const Purse>& senderCopy) const = 0;
#endif  // OT_CASH
    EXPORT virtual void Refresh() const = 0;
    EXPORT virtual std::uint64_t RefreshCount() const = 0;
    EXPORT virtual OTIdentifier RegisterNym(
        const Identifier& nymID,
        const Identifier& server,
        const bool setContactData) const = 0;
    EXPORT virtual OTIdentifier SetIntroductionServer(
        const ServerContract& contract) const = 0;
    EXPORT virtual OTIdentifier ScheduleDownloadAccount(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID) const = 0;
    EXPORT virtual OTIdentifier ScheduleDownloadContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& contractID) const = 0;
    EXPORT virtual OTIdentifier ScheduleDownloadNym(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID) const = 0;
    EXPORT virtual OTIdentifier ScheduleDownloadNymbox(
        const Identifier& localNymID,
        const Identifier& serverID) const = 0;
    EXPORT virtual OTIdentifier SchedulePublishServerContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& contractID) const = 0;
    EXPORT virtual OTIdentifier ScheduleRegisterAccount(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& unitID) const = 0;
    EXPORT virtual OTIdentifier ScheduleRegisterNym(
        const Identifier& localNymID,
        const Identifier& serverID) const = 0;
    EXPORT virtual OTIdentifier SendTransfer(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const int64_t value,
        const std::string& memo) const = 0;
    EXPORT virtual void StartIntroductionServer(
        const Identifier& localNymID) const = 0;
    EXPORT virtual ThreadStatus Status(const Identifier& thread) const = 0;

    EXPORT virtual ~Sync() = default;

protected:
    Sync() = default;

private:
    Sync(const Sync&) = delete;
    Sync(Sync&&) = delete;
    Sync& operator=(const Sync&) = delete;
    Sync& operator=(Sync&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CLIENT_SYNC_HPP
