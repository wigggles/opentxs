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

#ifndef OPENTXS_STORAGE_TREE_THREAD_HPP
#define OPENTXS_STORAGE_TREE_THREAD_HPP

#include "opentxs/api/Editor.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/storage/tree/Node.hpp"
#include "opentxs/storage/Storage.hpp"

#include <list>
#include <map>
#include <set>

namespace opentxs
{
namespace storage
{

class Mailbox;
class Threads;

class Thread : public Node
{
private:
    friend class Threads;
    typedef std::tuple<std::size_t, std::int64_t, std::string> SortKey;
    typedef std::map<SortKey, const proto::StorageThreadItem*> SortedItems;

    mutable std::string id_;
    std::string alias_;
    std::size_t index_{0};
    Mailbox& mail_inbox_;
    Mailbox& mail_outbox_;
    std::map<std::string, proto::StorageThreadItem> items_;

    // It's important to use a sorted container for this so the thread ID can be
    // calculated deterministically
    const std::set<std::string> participants_;

    void init(const std::string& hash) override;
    bool save(const std::unique_lock<std::mutex>& lock) const override;
    proto::StorageThread serialize(
        const std::unique_lock<std::mutex>& lock) const;
    SortedItems sort(const std::unique_lock<std::mutex>& lock) const;

    Thread(
        const StorageDriver& storage,
        const std::string& id,
        const std::string& hash,
        const std::string& alias,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
    Thread(
        const StorageDriver& storage,
        const std::set<std::string>& participants,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
    Thread() = delete;
    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread operator=(const Thread&) = delete;
    Thread operator=(Thread&&) = delete;

public:
    std::string Alias() const;
    bool Check(const std::string& id) const;
    std::string ID() const;
    proto::StorageThread Items() const;
    bool Migrate() const override;

    bool Add(
        const std::string& id,
        const std::uint64_t time,
        const StorageBox& box,
        const std::string& alias,
        const std::string& contents,
        const std::uint64_t index = 0,
        const std::string& account = std::string(""));
    bool Read(const std::string& id);
    bool Remove(const std::string& id);
    bool SetAlias(const std::string& alias);

    ~Thread() = default;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_THREAD_HPP
