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
 *  fellowtraveler\opentransactions.org
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

#ifndef OPENTXS_API_ACTIVITY_HPP
#define OPENTXS_API_ACTIVITY_HPP

#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"

#include <memory>
#include <string>

namespace opentxs
{

class ContactManager;
class Identifier;
class Message;
class OT;
class Storage;
class Wallet;

class Activity
{
public:
    /**   Load a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] id the identifier of the mail object
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    std::unique_ptr<Message> Mail(
        const Identifier& nym,
        const Identifier& id,
        const StorageBox& box) const;

    /**   Store a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] mail the mail object to be stored
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns The id of the stored message. The string will be empty if
     *             the mail object can not be stored.
     */
    std::string Mail(
        const Identifier& nym,
        const Message& mail,
        const StorageBox box);

    /**   Obtain a list of mail objects in a specified box
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] box the box to be listed
     */
    ObjectList Mail(const Identifier& nym, const StorageBox box) const;

    /**   Delete a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] mail the mail object to be stored
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns The id of the stored message. The string will be empty if
     *             the mail object can not be stored.
     */
    bool MailRemove(
        const Identifier& nym,
        const Identifier& id,
        const StorageBox box) const;

    std::shared_ptr<proto::StorageThread> Thread(
        const Identifier& nymID,
        const Identifier& threadID) const;

    /**   Obtain a list of thread ids for the specified nym
     *
     *    \param[in] nym the identifier of the nym
     */
    ObjectList Threads(const Identifier& nym) const;

    ~Activity() = default;

private:
    friend class OT;

    ContactManager& contact_;
    Storage& storage_;
    Wallet& wallet_;

    /**   Migrate nym-based thread IDs to contact-based thread IDs
     *
     *    This method should only be called by the ContactManager on startup
     */
    void MigrateLegacyThreads() const;

    std::string nym_to_contact(const std::string& nymID);

    Activity(ContactManager& contact, Storage& storage, Wallet& wallet);
    Activity() = delete;
    Activity(const Activity&) = delete;
    Activity(Activity&&) = delete;
    Activity operator=(const Activity&) = delete;
    Activity operator=(Activity&&) = delete;
};
}  // namespace opentxs
#endif  // OPENTXS_API_ACTIVITY_HPP
