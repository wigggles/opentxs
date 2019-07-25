// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class Activity final : virtual public api::client::internal::Activity, Lockable
{
public:
    bool AddBlockchainTransaction(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const std::string& txid,
        const Time time) const final;

    bool AddPaymentEvent(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const StorageBox type,
        const Identifier& itemID,
        const Identifier& workflowID,
        std::chrono::time_point<std::chrono::system_clock> time) const final;

    bool MoveIncomingBlockchainTransaction(
        const identifier::Nym& nymID,
        const Identifier& fromThreadID,
        const Identifier& toThreadID,
        const std::string& txid) const final;

    bool UnassignBlockchainTransaction(
        const identifier::Nym& nymID,
        const Identifier& fromThreadID,
        const std::string& txid) const final;

    /**   Load a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] id the identifier of the mail object
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    std::unique_ptr<Message> Mail(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box,
        const PasswordPrompt& reason) const final;

    /**   Store a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] mail the mail object to be stored
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns The id of the stored message. The string will be empty if
     *             the mail object can not be stored.
     */
    std::string Mail(
        const identifier::Nym& nym,
        const Message& mail,
        const StorageBox box,
        const PasswordPrompt& reason) const final;

    /**   Obtain a list of mail objects in a specified box
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] box the box to be listed
     */
    ObjectList Mail(const identifier::Nym& nym, const StorageBox box)
        const final;

    /**   Delete a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] mail the mail object to be stored
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns The id of the stored message. The string will be empty if
     *             the mail object can not be stored.
     */
    bool MailRemove(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box) const final;

    /**   Retrieve the text from a message
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] id the identifier of the mail object
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    std::shared_ptr<const std::string> MailText(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box,
        const PasswordPrompt& reason) const final;

    /**   Mark a thread item as read
     *
     *    \param[in] nymId the identifier of the nym who owns the thread
     *    \param[in] threadId the thread containing the item to be marked
     *    \param[in] itemId the identifier of the item to be marked read
     *    \returns False if the nym, thread, or item does not exist
     */
    bool MarkRead(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const final;

    /**   Mark a thread item as unread
     *
     *    \param[in] nymId the identifier of the nym who owns the thread
     *    \param[in] threadId the thread containing the item to be marked
     *    \param[in] itemId the identifier of the item to be marked unread
     *    \returns False if the nym, thread, or item does not exist
     */
    bool MarkUnread(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const final;

    ChequeData Cheque(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow,
        const PasswordPrompt& reason) const final;

    TransferData Transfer(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow,
        const PasswordPrompt& reason) const final;

    /**   Summarize a payment workflow event in human-friendly test form
     *
     *    \param[in] nym the identifier of the nym who owns the thread
     *    \param[in] id the identifier of the payment item
     *    \param[in] workflow the identifier of the payment workflow
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    std::shared_ptr<const std::string> PaymentText(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow,
        const PasswordPrompt& reason) const final;

    /**   Asynchronously cache the most recent items in each of a nym's threads
     *
     *    \param[in] nymID the identifier of the nym who owns the thread
     *    \param[in] count the number of items to preload in each thread
     */
    void PreloadActivity(
        const identifier::Nym& nymID,
        const std::size_t count,
        const PasswordPrompt& reason) const final;

    /**   Asynchronously cache the items in an activity thread
     *
     *    \param[in] nymID the identifier of the nym who owns the thread
     *    \param[in] threadID the thread containing the items to be cached
     *    \param[in] start the first item to be cached
     *    \param[in] count the number of items to cache
     */
    void PreloadThread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const std::size_t start,
        const std::size_t count,
        const PasswordPrompt& reason) const final;

    std::shared_ptr<proto::StorageThread> Thread(
        const identifier::Nym& nymID,
        const Identifier& threadID) const final;

    /**   Obtain a list of thread ids for the specified nym
     *
     *    \param[in] nym the identifier of the nym
     *    \param[in] unreadOnly if true, only return threads with unread items
     */
    ObjectList Threads(
        const identifier::Nym& nym,
        const PasswordPrompt& reason,
        const bool unreadOnly = false) const final;

    /**   Return the total number of unread thread items for a nym
     *
     *    \param[in] nymId
     */
    std::size_t UnreadCount(const identifier::Nym& nym) const final;

    std::string ThreadPublisher(const identifier::Nym& nym) const final;

    ~Activity() = default;

private:
    friend opentxs::Factory;

    typedef std::map<OTIdentifier, std::shared_ptr<const std::string>>
        MailCache;

    const api::Core& api_;
    const client::Contacts& contact_;
    mutable std::mutex mail_cache_lock_;
    mutable MailCache mail_cache_;
    mutable std::mutex publisher_lock_;
    mutable std::map<OTIdentifier, OTZMQPublishSocket> thread_publishers_;

    /**   Migrate nym-based thread IDs to contact-based thread IDs
     *
     *    This method should only be called by the Contacts on startup
     */
    void MigrateLegacyThreads(const PasswordPrompt& reason) const final;
    void activity_preload_thread(
        OTPasswordPrompt reason,
        const OTIdentifier nymID,
        const std::size_t count) const;
    void preload(
        OTPasswordPrompt reason,
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box) const;
    void thread_preload_thread(
        OTPasswordPrompt reason,
        const std::string nymID,
        const std::string threadID,
        const std::size_t start,
        const std::size_t count) const;

    std::shared_ptr<const Contact> nym_to_contact(
        const PasswordPrompt& reason,
        const std::string& nymID) const;
    const opentxs::network::zeromq::socket::Publish& get_publisher(
        const identifier::Nym& nymID) const;
    const opentxs::network::zeromq::socket::Publish& get_publisher(
        const identifier::Nym& nymID,
        std::string& endpoint) const;
    void publish(const identifier::Nym& nymID, const std::string& threadID)
        const;

    Activity(const api::Core& api, const client::Contacts& contact);
    Activity() = delete;
    Activity(const Activity&) = delete;
    Activity(Activity&&) = delete;
    Activity& operator=(const Activity&) = delete;
    Activity& operator=(Activity&&) = delete;
};
}  // namespace opentxs::api::client::implementation
