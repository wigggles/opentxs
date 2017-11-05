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

#ifndef OPENTXS_STORAGE_TREE_NYM_HPP
#define OPENTXS_STORAGE_TREE_NYM_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/storage/tree/Contexts.hpp"
#include "opentxs/storage/tree/Mailbox.hpp"
#include "opentxs/storage/tree/Node.hpp"
#include "opentxs/storage/tree/PeerReplies.hpp"
#include "opentxs/storage/tree/PeerRequests.hpp"
#include "opentxs/storage/tree/Thread.hpp"
#include "opentxs/storage/tree/Threads.hpp"

#include <cstdint>
#include <map>
#include <set>
#include <string>

namespace opentxs
{
namespace storage
{
class Nyms;

class Nym : public Node
{
private:
    friend class Nyms;

    std::string alias_;
    std::string nymid_;
    std::string credentials_;

    mutable std::atomic<bool> checked_;
    mutable std::atomic<bool> private_;
    mutable std::atomic<std::uint64_t> revision_;

    mutable std::mutex sent_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> sent_request_box_;
    std::string sent_peer_request_;
    mutable std::mutex incoming_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> incoming_request_box_;
    std::string incoming_peer_request_;
    mutable std::mutex sent_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> sent_reply_box_;
    std::string sent_peer_reply_;
    mutable std::mutex incoming_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> incoming_reply_box_;
    std::string incoming_peer_reply_;
    mutable std::mutex finished_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> finished_request_box_;
    std::string finished_peer_request_;
    mutable std::mutex finished_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> finished_reply_box_;
    std::string finished_peer_reply_;
    mutable std::mutex processed_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> processed_request_box_;
    std::string processed_peer_request_;
    mutable std::mutex processed_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> processed_reply_box_;
    std::string processed_peer_reply_;
    mutable std::mutex mail_inbox_lock_;
    mutable std::unique_ptr<Mailbox> mail_inbox_;
    std::string mail_inbox_root_;
    mutable std::mutex mail_outbox_lock_;
    mutable std::unique_ptr<Mailbox> mail_outbox_;
    std::string mail_outbox_root_;
    mutable std::mutex threads_lock_;
    mutable std::unique_ptr<class Threads> threads_;
    std::string threads_root_;
    mutable std::mutex contexts_lock_;
    mutable std::unique_ptr<class Contexts> contexts_;
    std::string contexts_root_;
    mutable std::mutex blockchain_lock_;
    std::map<proto::ContactItemType, std::set<std::string>>
        blockchain_account_types_{};
    std::map<std::string, std::shared_ptr<proto::Bip44Account>>
        blockchain_accounts_{};

    PeerRequests* sent_request_box() const;
    PeerRequests* incoming_request_box() const;
    PeerReplies* sent_reply_box() const;
    PeerReplies* incoming_reply_box() const;
    PeerRequests* finished_request_box() const;
    PeerReplies* finished_reply_box() const;
    PeerRequests* processed_request_box() const;
    PeerReplies* processed_reply_box() const;
    Mailbox* mail_inbox() const;
    Mailbox* mail_outbox() const;
    class Threads* threads() const;
    class Contexts* contexts() const;

    void save(
        PeerReplies* input,
        const std::unique_lock<std::mutex>& lock,
        StorageBox type);
    void save(
        PeerRequests* input,
        const std::unique_lock<std::mutex>& lock,
        StorageBox type);
    void save(
        Mailbox* input,
        const std::unique_lock<std::mutex>& lock,
        StorageBox type);
    void save(class Threads* input, const std::unique_lock<std::mutex>& lock);
    void save(class Contexts* input, const std::unique_lock<std::mutex>& lock);

    void init(const std::string& hash) override;
    bool save(const std::unique_lock<std::mutex>& lock) const override;
    void update_hash(const StorageBox type, const std::string& root);
    proto::StorageNym serialize() const;

    Nym(const StorageDriver& storage,
        const std::string& id,
        const std::string& hash,
        const std::string& alias);
    Nym() = delete;
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    Nym operator=(const Nym&) = delete;
    Nym operator=(Nym&&) = delete;

public:
    const PeerRequests& SentRequestBox() const;
    const PeerRequests& IncomingRequestBox() const;
    const PeerReplies& SentReplyBox() const;
    const PeerReplies& IncomingReplyBox() const;
    const PeerRequests& FinishedRequestBox() const;
    const PeerReplies& FinishedReplyBox() const;
    const PeerRequests& ProcessedRequestBox() const;
    const PeerReplies& ProcessedReplyBox() const;
    const Mailbox& MailInbox() const;
    const Mailbox& MailOutbox() const;
    const class Threads& Threads() const;
    const class Contexts& Contexts() const;
    std::set<std::string> BlockchainAccountList(
        const proto::ContactItemType type) const;

    Editor<PeerRequests> mutable_SentRequestBox();
    Editor<PeerRequests> mutable_IncomingRequestBox();
    Editor<PeerReplies> mutable_SentReplyBox();
    Editor<PeerReplies> mutable_IncomingReplyBox();
    Editor<PeerRequests> mutable_FinishedRequestBox();
    Editor<PeerReplies> mutable_FinishedReplyBox();
    Editor<PeerRequests> mutable_ProcessedRequestBox();
    Editor<PeerReplies> mutable_ProcessedReplyBox();
    Editor<Mailbox> mutable_MailInbox();
    Editor<Mailbox> mutable_MailOutbox();
    Editor<class Threads> mutable_Threads();
    Editor<class Contexts> mutable_Contexts();

    std::string Alias() const;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Bip44Account>& output,
        const bool checking) const;
    bool Load(
        std::shared_ptr<proto::CredentialIndex>& output,
        std::string& alias,
        const bool checking) const;
    bool Migrate(const StorageDriver& to) const override;

    bool SetAlias(const std::string& alias);
    bool Store(
        const proto::ContactItemType type,
        const proto::Bip44Account& data);
    bool Store(
        const proto::CredentialIndex& data,
        const std::string& alias,
        std::string& plaintext);

    ~Nym() = default;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_NYM_HPP
