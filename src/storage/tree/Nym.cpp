/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Nym, CLI, GUI
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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/storage/tree/Nym.hpp"

#include "opentxs/storage/StoragePlugin.hpp"

#include <functional>

namespace opentxs
{
namespace storage
{
Nym::Nym(
    const StorageDriver& storage,
    const std::string& id,
    const std::string& hash,
    const std::string& alias)
    : Node(storage, hash)
    , alias_(alias)
    , nymid_(id)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = 3;
        root_ = Node::BLANK_HASH;
        credentials_ = Node::BLANK_HASH;
        sent_peer_request_ = Node::BLANK_HASH;
        incoming_peer_request_ = Node::BLANK_HASH;
        sent_peer_reply_ = Node::BLANK_HASH;
        incoming_peer_reply_ = Node::BLANK_HASH;
        finished_peer_request_ = Node::BLANK_HASH;
        finished_peer_reply_ = Node::BLANK_HASH;
        processed_peer_request_ = Node::BLANK_HASH;
        processed_peer_reply_ = Node::BLANK_HASH;
        mail_inbox_root_ = Node::BLANK_HASH;
        mail_outbox_root_ = Node::BLANK_HASH;
        threads_root_ = Node::BLANK_HASH;
        contexts_root_ = Node::BLANK_HASH;
    }

    checked_.store(false);
    private_.store(false);
    revision_.store(0);
}

std::string Nym::Alias() const { return alias_; }

class Contexts* Nym::contexts() const
{
    std::unique_lock<std::mutex> lock(contexts_lock_);

    if (!contexts_) {
        contexts_.reset(new class Contexts(driver_, contexts_root_));

        if (!contexts_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return contexts_.get();
}

const class Contexts& Nym::Contexts() const { return *contexts(); }

PeerReplies* Nym::finished_reply_box() const
{
    std::unique_lock<std::mutex> lock(finished_reply_box_lock_);

    if (!finished_reply_box_) {
        finished_reply_box_.reset(
            new PeerReplies(driver_, finished_peer_reply_));

        if (!finished_reply_box_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return finished_reply_box_.get();
}

PeerRequests* Nym::finished_request_box() const
{
    std::unique_lock<std::mutex> lock(finished_request_box_lock_);

    if (!finished_request_box_) {
        finished_request_box_.reset(
            new PeerRequests(driver_, finished_peer_request_));

        if (!finished_request_box_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return finished_request_box_.get();
}

const PeerRequests& Nym::FinishedRequestBox() const
{
    return *finished_request_box();
}

const PeerReplies& Nym::FinishedReplyBox() const
{
    return *finished_reply_box();
}

PeerReplies* Nym::incoming_reply_box() const
{
    std::unique_lock<std::mutex> lock(incoming_reply_box_lock_);

    if (!incoming_reply_box_) {
        incoming_reply_box_.reset(
            new PeerReplies(driver_, incoming_peer_reply_));

        if (!incoming_reply_box_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return incoming_reply_box_.get();
}

PeerRequests* Nym::incoming_request_box() const
{
    std::unique_lock<std::mutex> lock(incoming_request_box_lock_);

    if (!incoming_request_box_) {
        incoming_request_box_.reset(
            new PeerRequests(driver_, incoming_peer_request_));

        if (!incoming_request_box_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return incoming_request_box_.get();
}

const PeerRequests& Nym::IncomingRequestBox() const
{
    return *incoming_request_box();
}

const PeerReplies& Nym::IncomingReplyBox() const
{
    return *incoming_reply_box();
}

void Nym::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNym> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load nym index file."
                  << std::endl;
        abort();
    }

    version_ = serialized->version();

    // Upgrade to version 3
    if (3 > version_) {
        version_ = 3;
    }

    nymid_ = serialized->nymid();
    credentials_ = serialized->credlist().hash();
    sent_peer_request_ = serialized->sentpeerrequests().hash();
    incoming_peer_request_ = serialized->incomingpeerrequests().hash();
    sent_peer_reply_ = serialized->sentpeerreply().hash();
    incoming_peer_reply_ = serialized->incomingpeerreply().hash();
    finished_peer_request_ = serialized->finishedpeerrequest().hash();
    finished_peer_reply_ = serialized->finishedpeerreply().hash();
    processed_peer_request_ = serialized->processedpeerrequest().hash();
    processed_peer_reply_ = serialized->processedpeerreply().hash();

    // Fields added in version 2
    if (serialized->has_mailinbox()) {
        mail_inbox_root_ = serialized->mailinbox().hash();
    } else {
        mail_inbox_root_ = Node::BLANK_HASH;
    }

    if (serialized->has_mailoutbox()) {
        mail_outbox_root_ = serialized->mailoutbox().hash();
    } else {
        mail_outbox_root_ = Node::BLANK_HASH;
    }

    if (serialized->has_threads()) {
        threads_root_ = serialized->threads().hash();
    } else {
        threads_root_ = Node::BLANK_HASH;
    }

    if (serialized->has_contexts()) {
        contexts_root_ = serialized->contexts().hash();
    } else {
        contexts_root_ = Node::BLANK_HASH;
    }
}

bool Nym::Load(
    std::shared_ptr<proto::CredentialIndex>& output,
    std::string& alias,
    const bool checking) const
{
    std::lock_guard<std::mutex> lock(write_lock_);

    if (!check_hash(credentials_)) {
        if (!checking) {
            std::cerr << __FUNCTION__ << ": Error: nym with id " << nymid_
                      << " does not exist." << std::endl;
        }

        return false;
    }

    alias = alias_;
    checked_.store(driver_.LoadProto(credentials_, output, false));

    if (!checked_.load()) {
        return false;
    }

    private_.store(proto::CREDINDEX_PRIVATE == output->mode());
    revision_.store(output->revision());

    return true;
}

Mailbox* Nym::mail_inbox() const
{
    std::unique_lock<std::mutex> lock(mail_inbox_lock_);

    if (!mail_inbox_) {
        mail_inbox_.reset(new Mailbox(driver_, mail_inbox_root_));

        if (!mail_inbox_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return mail_inbox_.get();
}

Mailbox* Nym::mail_outbox() const
{
    std::unique_lock<std::mutex> lock(mail_outbox_lock_);

    if (!mail_outbox_) {
        mail_outbox_.reset(new Mailbox(driver_, mail_outbox_root_));

        if (!mail_outbox_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return mail_outbox_.get();
}

const Mailbox& Nym::MailInbox() const { return *mail_inbox(); }

const Mailbox& Nym::MailOutbox() const { return *mail_outbox(); }

bool Nym::Migrate(const StorageDriver& to) const
{
    bool output{true};
    output &= migrate(credentials_, to);
    output &= sent_request_box()->Migrate(to);
    output &= incoming_request_box()->Migrate(to);
    output &= sent_reply_box()->Migrate(to);
    output &= incoming_reply_box()->Migrate(to);
    output &= finished_request_box()->Migrate(to);
    output &= finished_reply_box()->Migrate(to);
    output &= processed_request_box()->Migrate(to);
    output &= processed_reply_box()->Migrate(to);
    output &= mail_inbox()->Migrate(to);
    output &= mail_outbox()->Migrate(to);
    output &= threads()->Migrate(to);
    output &= contexts()->Migrate(to);
    output &= migrate(root_, to);

    return output;
}

Editor<PeerRequests> Nym::mutable_SentRequestBox()
{
    std::function<void(PeerRequests*, std::unique_lock<std::mutex>&)> callback =
        [&](PeerRequests* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, StorageBox::SENTPEERREQUEST);
    };

    return Editor<PeerRequests>(write_lock_, sent_request_box(), callback);
}

Editor<PeerRequests> Nym::mutable_IncomingRequestBox()
{
    std::function<void(PeerRequests*, std::unique_lock<std::mutex>&)> callback =
        [&](PeerRequests* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, StorageBox::INCOMINGPEERREQUEST);
    };

    return Editor<PeerRequests>(write_lock_, incoming_request_box(), callback);
}

Editor<PeerReplies> Nym::mutable_SentReplyBox()
{
    std::function<void(PeerReplies*, std::unique_lock<std::mutex>&)> callback =
        [&](PeerReplies* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, StorageBox::SENTPEERREPLY);
    };

    return Editor<PeerReplies>(write_lock_, sent_reply_box(), callback);
}

Editor<PeerReplies> Nym::mutable_IncomingReplyBox()
{
    std::function<void(PeerReplies*, std::unique_lock<std::mutex>&)> callback =
        [&](PeerReplies* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, StorageBox::INCOMINGPEERREPLY);
    };

    return Editor<PeerReplies>(write_lock_, incoming_reply_box(), callback);
}

Editor<PeerRequests> Nym::mutable_FinishedRequestBox()
{
    std::function<void(PeerRequests*, std::unique_lock<std::mutex>&)> callback =
        [&](PeerRequests* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, StorageBox::FINISHEDPEERREQUEST);
    };

    return Editor<PeerRequests>(write_lock_, finished_request_box(), callback);
}

Editor<PeerReplies> Nym::mutable_FinishedReplyBox()
{
    std::function<void(PeerReplies*, std::unique_lock<std::mutex>&)> callback =
        [&](PeerReplies* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, StorageBox::FINISHEDPEERREPLY);
    };

    return Editor<PeerReplies>(write_lock_, finished_reply_box(), callback);
}

Editor<PeerRequests> Nym::mutable_ProcessedRequestBox()
{
    std::function<void(PeerRequests*, std::unique_lock<std::mutex>&)> callback =
        [&](PeerRequests* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, StorageBox::PROCESSEDPEERREQUEST);
    };

    return Editor<PeerRequests>(write_lock_, processed_request_box(), callback);
}

Editor<PeerReplies> Nym::mutable_ProcessedReplyBox()
{
    std::function<void(PeerReplies*, std::unique_lock<std::mutex>&)> callback =
        [&](PeerReplies* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, StorageBox::PROCESSEDPEERREPLY);
    };

    return Editor<PeerReplies>(write_lock_, processed_reply_box(), callback);
}

Editor<Mailbox> Nym::mutable_MailInbox()
{
    std::function<void(Mailbox*, std::unique_lock<std::mutex>&)> callback =
        [&](Mailbox* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, StorageBox::MAILINBOX);
    };

    return Editor<Mailbox>(write_lock_, mail_inbox(), callback);
}

Editor<Mailbox> Nym::mutable_MailOutbox()
{
    std::function<void(Mailbox*, std::unique_lock<std::mutex>&)> callback =
        [&](Mailbox* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, StorageBox::MAILOUTBOX);
    };

    return Editor<Mailbox>(write_lock_, mail_outbox(), callback);
}

Editor<class Threads> Nym::mutable_Threads()
{
    std::function<void(class Threads*, std::unique_lock<std::mutex>&)>
        callback =
            [&](class Threads* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock);
    };

    return Editor<class Threads>(write_lock_, threads(), callback);
}

Editor<class Contexts> Nym::mutable_Contexts()
{
    std::function<void(class Contexts*, std::unique_lock<std::mutex>&)>
        callback = [&](
            class Contexts* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock);
    };

    return Editor<class Contexts>(write_lock_, contexts(), callback);
}

PeerReplies* Nym::processed_reply_box() const
{
    std::unique_lock<std::mutex> lock(processed_reply_box_lock_);

    if (!processed_reply_box_) {
        processed_reply_box_.reset(
            new PeerReplies(driver_, processed_peer_reply_));

        if (!processed_reply_box_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return processed_reply_box_.get();
}

PeerRequests* Nym::processed_request_box() const
{
    std::unique_lock<std::mutex> lock(processed_request_box_lock_);

    if (!processed_request_box_) {
        processed_request_box_.reset(
            new PeerRequests(driver_, processed_peer_request_));

        if (!processed_request_box_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return processed_request_box_.get();
}

const PeerRequests& Nym::ProcessedRequestBox() const
{
    return *processed_request_box();
}

const PeerReplies& Nym::ProcessedReplyBox() const
{
    return *processed_reply_box();
}

bool Nym::save(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) {
        return false;
    }

    return driver_.StoreProto(serialized, root_);
}

void Nym::save(
    PeerReplies* input,
    const std::unique_lock<std::mutex>& lock,
    StorageBox type)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == input) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    update_hash(type, input->Root());

    if (!save(lock)) {
        std::cerr << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Nym::save(
    PeerRequests* input,
    const std::unique_lock<std::mutex>& lock,
    StorageBox type)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == input) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    update_hash(type, input->Root());

    if (!save(lock)) {
        std::cerr << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Nym::save(
    Mailbox* input,
    const std::unique_lock<std::mutex>& lock,
    StorageBox type)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == input) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    update_hash(type, input->Root());

    if (!save(lock)) {
        std::cerr << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Nym::save(class Threads* input, const std::unique_lock<std::mutex>& lock)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == input) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    if (mail_inbox_) {
        update_hash(StorageBox::MAILINBOX, mail_inbox_->Root());
    }

    if (mail_outbox_) {
        update_hash(StorageBox::MAILOUTBOX, mail_outbox_->Root());
    }

    threads_root_ = input->Root();

    if (!save(lock)) {
        std::cerr << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

void Nym::save(class Contexts* input, const std::unique_lock<std::mutex>& lock)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == input) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    contexts_root_ = input->Root();

    if (!save(lock)) {
        std::cerr << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

class Threads* Nym::threads() const
{
    std::unique_lock<std::mutex> lock(threads_lock_);

    if (!threads_) {
        threads_.reset(new class Threads(
            driver_, threads_root_, *mail_inbox(), *mail_outbox()));

        if (!threads_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return threads_.get();
}

const class Threads& Nym::Threads() const { return *threads(); }

void Nym::update_hash(const StorageBox type, const std::string& root)
{
    switch (type) {
        case StorageBox::SENTPEERREQUEST: {
            std::lock_guard<std::mutex> lock(sent_request_box_lock_);
            sent_peer_request_ = root;
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            std::lock_guard<std::mutex> lock(incoming_request_box_lock_);
            incoming_peer_request_ = root;
        } break;
        case StorageBox::SENTPEERREPLY: {
            std::lock_guard<std::mutex> lock(sent_reply_box_lock_);
            sent_peer_reply_ = root;
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            std::lock_guard<std::mutex> lock(incoming_reply_box_lock_);
            incoming_peer_reply_ = root;
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            std::lock_guard<std::mutex> lock(finished_request_box_lock_);
            finished_peer_request_ = root;
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            std::lock_guard<std::mutex> lock(finished_reply_box_lock_);
            finished_peer_reply_ = root;
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            std::lock_guard<std::mutex> lock(finished_reply_box_lock_);
            processed_peer_request_ = root;
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            std::lock_guard<std::mutex> lock(processed_reply_box_lock_);
            processed_peer_reply_ = root;
        } break;
        case StorageBox::MAILINBOX: {
            std::lock_guard<std::mutex> lock(mail_inbox_lock_);
            mail_inbox_root_ = root;
        } break;
        case StorageBox::MAILOUTBOX: {
            std::lock_guard<std::mutex> lock(mail_outbox_lock_);
            mail_outbox_root_ = root;
        } break;
        default: {
            std::cerr << __FUNCTION__ << ": Unknown box" << std::endl;
            abort();
        }
    }
}

PeerReplies* Nym::sent_reply_box() const
{
    std::unique_lock<std::mutex> lock(sent_reply_box_lock_);

    if (!sent_reply_box_) {
        sent_reply_box_.reset(new PeerReplies(driver_, sent_peer_reply_));

        if (!sent_reply_box_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return sent_reply_box_.get();
}

PeerRequests* Nym::sent_request_box() const
{
    std::unique_lock<std::mutex> lock(sent_request_box_lock_);

    if (!sent_request_box_) {
        sent_request_box_.reset(new PeerRequests(driver_, sent_peer_request_));

        if (!sent_request_box_) {
            std::cerr << __FUNCTION__ << ": Unable to instantiate."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return sent_request_box_.get();
}

const PeerRequests& Nym::SentRequestBox() const { return *sent_request_box(); }

const PeerReplies& Nym::SentReplyBox() const { return *sent_reply_box(); }

proto::StorageNym Nym::serialize() const
{
    proto::StorageNym serialized;
    serialized.set_version(version_);
    serialized.set_nymid(nymid_);

    set_hash(version_, nymid_, credentials_, *serialized.mutable_credlist());
    set_hash(
        version_,
        nymid_,
        sent_peer_request_,
        *serialized.mutable_sentpeerrequests());
    set_hash(
        version_,
        nymid_,
        incoming_peer_request_,
        *serialized.mutable_incomingpeerrequests());
    set_hash(
        version_,
        nymid_,
        sent_peer_reply_,
        *serialized.mutable_sentpeerreply());
    set_hash(
        version_,
        nymid_,
        incoming_peer_reply_,
        *serialized.mutable_incomingpeerreply());
    set_hash(
        version_,
        nymid_,
        finished_peer_request_,
        *serialized.mutable_finishedpeerrequest());
    set_hash(
        version_,
        nymid_,
        finished_peer_reply_,
        *serialized.mutable_finishedpeerreply());
    set_hash(
        version_,
        nymid_,
        processed_peer_request_,
        *serialized.mutable_processedpeerrequest());
    set_hash(
        version_,
        nymid_,
        processed_peer_reply_,
        *serialized.mutable_processedpeerreply());
    set_hash(
        version_, nymid_, mail_inbox_root_, *serialized.mutable_mailinbox());
    set_hash(
        version_, nymid_, mail_outbox_root_, *serialized.mutable_mailoutbox());
    set_hash(version_, nymid_, threads_root_, *serialized.mutable_threads());
    set_hash(version_, nymid_, contexts_root_, *serialized.mutable_contexts());

    return serialized;
}

bool Nym::SetAlias(const std::string& alias)
{
    std::lock_guard<std::mutex> lock(write_lock_);

    alias_ = alias;

    return true;
}

bool Nym::Store(
    const proto::CredentialIndex& data,
    const std::string& alias,
    std::string& plaintext)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    const std::uint64_t revision = data.revision();
    bool saveOk = false;
    const bool incomingPublic = (proto::CREDINDEX_PUBLIC == data.mode());
    const bool existing = check_hash(credentials_);

    if (existing) {
        if (incomingPublic) {
            if (checked_.load()) {
                saveOk = !private_.load();
            } else {
                std::shared_ptr<proto::CredentialIndex> serialized;
                driver_.LoadProto(credentials_, serialized, true);
                saveOk = !private_.load();
            }
        } else {
            saveOk = true;
        }
    } else {
        saveOk = true;
    }

    const bool keyUpgrade = (!incomingPublic) && (!private_.load());
    const bool revisionUpgrade = revision > revision_.load();
    const bool upgrade = keyUpgrade || revisionUpgrade;

    if (saveOk) {
        if (upgrade) {
            const bool saved = driver_.StoreProto<proto::CredentialIndex>(
                data, credentials_, plaintext);

            if (!saved) {
                return false;
            }

            revision_.store(revision);

            if (!alias.empty()) {
                alias_ = alias;
            }
        }
    }

    checked_.store(true);
    private_.store(!incomingPublic);

    return save(lock);
}
}  // namespace storage
}  // namespace opentxs
