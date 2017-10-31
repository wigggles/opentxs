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

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace opentxs
{

class Contact;
class ContactManager;
class Identifier;
class Message;
class OT;
class Storage;
class Wallet;

class Activity
{
public:
    bool AddBlockchainTransaction(
        const Identifier& nymID,
        const Identifier& threadID,
        const StorageBox box,
        const proto::BlockchainTransaction& transaction) const;

    bool MoveIncomingBlockchainTransaction(
        const Identifier& nymID,
        const Identifier& fromThreadID,
        const Identifier& toThreadID,
        const std::string& txid) const;

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
        const StorageBox box) const;

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

    /**   Retrieve the text from a message
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] id the identifier of the mail object
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    std::shared_ptr<const std::string> MailText(
        const Identifier& nym,
        const Identifier& id,
        const StorageBox& box) const;

    /**   Mark a thread item as read
     *
     *    \param[in] nymId the identifier of the nym who owns the thread
     *    \param[in] threadId the thread containing the item to be marked
     *    \param[in] itemId the identifier of the item to be marked read
     *    \returns False if the nym, thread, or item does not exist
     */
    bool MarkRead(
        const Identifier& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const;

    /**   Mark a thread item as unread
     *
     *    \param[in] nymId the identifier of the nym who owns the thread
     *    \param[in] threadId the thread containing the item to be marked
     *    \param[in] itemId the identifier of the item to be marked unread
     *    \returns False if the nym, thread, or item does not exist
     */
    bool MarkUnread(
        const Identifier& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const;

    /**   Asynchronously cache the most recent items in each of a nym's threads
     *
     *    \param[in] nymID the identifier of the nym who owns the thread
     *    \param[in] count the number of items to preload in each thread
     */
    void PreloadActivity(const Identifier& nymID, const std::size_t count)
        const;

    /**   Asynchronously cache the items in an activity thread
     *
     *    \param[in] nymID the identifier of the nym who owns the thread
     *    \param[in] threadID the thread containing the items to be cached
     *    \param[in] start the first item to be cached
     *    \param[in] count the number of items to cache
     */
    void PreloadThread(
        const Identifier& nymID,
        const Identifier& threadID,
        const std::size_t start,
        const std::size_t count) const;

    std::shared_ptr<proto::StorageThread> Thread(
        const Identifier& nymID,
        const Identifier& threadID) const;

    /**   Obtain a list of thread ids for the specified nym
     *
     *    \param[in] nym the identifier of the nym
     *    \param[in] unreadOnly if true, only return threads with unread items
     */
    ObjectList Threads(const Identifier& nym, const bool unreadOnly = false)
        const;

    /**   Return the total number of unread thread items for a nym
     *
     *    \param[in] nymId
     */
    std::size_t UnreadCount(const Identifier& nym) const;

    ~Activity() = default;

private:
    friend class OT;

    typedef std::map<Identifier, std::shared_ptr<const std::string>> MailCache;

    ContactManager& contact_;
    Storage& storage_;
    Wallet& wallet_;
    mutable std::mutex mail_cache_lock_;
    mutable MailCache mail_cache_;

    /**   Migrate nym-based thread IDs to contact-based thread IDs
     *
     *    This method should only be called by the ContactManager on startup
     */
    void MigrateLegacyThreads() const;
    void activity_preload_thread(
        const Identifier nymID,
        const std::size_t count) const;
    void preload(
        const Identifier nym,
        const Identifier id,
        const StorageBox box) const;
    void thread_preload_thread(
        const std::string nymID,
        const std::string threadID,
        const std::size_t start,
        const std::size_t count) const;

    std::shared_ptr<const Contact> nym_to_contact(
        const std::string& nymID) const;

    Activity(ContactManager& contact, Storage& storage, Wallet& wallet);
    Activity() = delete;
    Activity(const Activity&) = delete;
    Activity(Activity&&) = delete;
    Activity operator=(const Activity&) = delete;
    Activity operator=(Activity&&) = delete;
};
}  // namespace opentxs
#endif  // OPENTXS_API_ACTIVITY_HPP
