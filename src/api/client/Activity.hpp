// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/client/Activity.cpp"

#pragma once

#include <chrono>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "internal/api/client/Client.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Contacts;
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class StorageThread;
}  // namespace proto

class Contact;
class Factory;
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
class Activity final : virtual public api::client::internal::Activity, Lockable
{
public:
    auto AddBlockchainTransaction(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const std::string& txid,
        const Time time) const -> bool final;

    auto AddPaymentEvent(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const StorageBox type,
        const Identifier& itemID,
        const Identifier& workflowID,
        std::chrono::time_point<std::chrono::system_clock> time) const
        -> bool final;

    auto MoveIncomingBlockchainTransaction(
        const identifier::Nym& nymID,
        const Identifier& fromThreadID,
        const Identifier& toThreadID,
        const std::string& txid) const -> bool final;

    auto UnassignBlockchainTransaction(
        const identifier::Nym& nymID,
        const Identifier& fromThreadID,
        const std::string& txid) const -> bool final;

    /**   Load a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] id the identifier of the mail object
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    auto Mail(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box) const -> std::unique_ptr<Message> final;

    /**   Store a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] mail the mail object to be stored
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns The id of the stored message. The string will be empty if
     *             the mail object can not be stored.
     */
    auto Mail(
        const identifier::Nym& nym,
        const Message& mail,
        const StorageBox box,
        const PasswordPrompt& reason) const -> std::string final;

    /**   Obtain a list of mail objects in a specified box
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] box the box to be listed
     */
    auto Mail(const identifier::Nym& nym, const StorageBox box) const
        -> ObjectList final;

    /**   Delete a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] mail the mail object to be stored
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns The id of the stored message. The string will be empty if
     *             the mail object can not be stored.
     */
    auto MailRemove(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box) const -> bool final;

    /**   Retrieve the text from a message
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] id the identifier of the mail object
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    auto MailText(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box,
        const PasswordPrompt& reason) const
        -> std::shared_ptr<const std::string> final;

    /**   Mark a thread item as read
     *
     *    \param[in] nymId the identifier of the nym who owns the thread
     *    \param[in] threadId the thread containing the item to be marked
     *    \param[in] itemId the identifier of the item to be marked read
     *    \returns False if the nym, thread, or item does not exist
     */
    auto MarkRead(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const -> bool final;

    /**   Mark a thread item as unread
     *
     *    \param[in] nymId the identifier of the nym who owns the thread
     *    \param[in] threadId the thread containing the item to be marked
     *    \param[in] itemId the identifier of the item to be marked unread
     *    \returns False if the nym, thread, or item does not exist
     */
    auto MarkUnread(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const -> bool final;

    auto Cheque(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const -> ChequeData final;

    auto Transfer(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const -> TransferData final;

    /**   Summarize a payment workflow event in human-friendly test form
     *
     *    \param[in] nym the identifier of the nym who owns the thread
     *    \param[in] id the identifier of the payment item
     *    \param[in] workflow the identifier of the payment workflow
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    auto PaymentText(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const
        -> std::shared_ptr<const std::string> final;

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

    auto Thread(const identifier::Nym& nymID, const Identifier& threadID) const
        -> std::shared_ptr<proto::StorageThread> final;

    /**   Obtain a list of thread ids for the specified nym
     *
     *    \param[in] nym the identifier of the nym
     *    \param[in] unreadOnly if true, only return threads with unread items
     */
    auto Threads(const identifier::Nym& nym, const bool unreadOnly = false)
        const -> ObjectList final;

    /**   Return the total number of unread thread items for a nym
     *
     *    \param[in] nymId
     */
    auto UnreadCount(const identifier::Nym& nym) const -> std::size_t final;

    auto ThreadPublisher(const identifier::Nym& nym) const -> std::string final;

    ~Activity() = default;

private:
    friend opentxs::Factory;

    typedef std::map<OTIdentifier, std::shared_ptr<const std::string>>
        MailCache;

    const api::internal::Core& api_;
    const client::Contacts& contact_;
    mutable std::mutex mail_cache_lock_;
    mutable MailCache mail_cache_;
    mutable std::mutex publisher_lock_;
    mutable std::map<OTIdentifier, OTZMQPublishSocket> thread_publishers_;

    /**   Migrate nym-based thread IDs to contact-based thread IDs
     *
     *    This method should only be called by the Contacts on startup
     */
    void MigrateLegacyThreads() const final;
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

    auto nym_to_contact(const std::string& nymID) const
        -> std::shared_ptr<const Contact>;
    auto get_publisher(const identifier::Nym& nymID) const
        -> const opentxs::network::zeromq::socket::Publish&;
    auto get_publisher(const identifier::Nym& nymID, std::string& endpoint)
        const -> const opentxs::network::zeromq::socket::Publish&;
    void publish(const identifier::Nym& nymID, const std::string& threadID)
        const;

    Activity(const api::internal::Core& api, const client::Contacts& contact);
    Activity() = delete;
    Activity(const Activity&) = delete;
    Activity(Activity&&) = delete;
    auto operator=(const Activity&) -> Activity& = delete;
    auto operator=(Activity &&) -> Activity& = delete;
};
}  // namespace opentxs::api::client::implementation
