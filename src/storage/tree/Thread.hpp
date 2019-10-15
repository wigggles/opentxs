// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include "Node.hpp"

#include <list>
#include <map>
#include <set>

namespace opentxs
{
namespace storage
{
class Thread final : public Node
{
private:
    friend class Threads;
    typedef std::tuple<std::size_t, std::int64_t, std::string> SortKey;
    typedef std::map<SortKey, const proto::StorageThreadItem*> SortedItems;

    std::string id_;
    std::string alias_;
    std::size_t index_;
    Mailbox& mail_inbox_;
    Mailbox& mail_outbox_;
    std::map<std::string, proto::StorageThreadItem> items_;
    // It's important to use a sorted container for this so the thread ID can be
    // calculated deterministically
    std::set<std::string> participants_;

    void init(const std::string& hash) final;
    bool save(const Lock& lock) const final;
    proto::StorageThread serialize(const Lock& lock) const;
    SortedItems sort(const Lock& lock) const;
    void upgrade(const Lock& lock);

    Thread(
        const opentxs::api::storage::Driver& storage,
        const std::string& id,
        const std::string& hash,
        const std::string& alias,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
    Thread(
        const opentxs::api::storage::Driver& storage,
        const std::string& id,
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
    bool Migrate(const opentxs::api::storage::Driver& to) const final;
    std::size_t UnreadCount() const;

    bool Add(
        const std::string& id,
        const std::uint64_t time,
        const StorageBox& box,
        const std::string& alias,
        const std::string& contents,
        const std::uint64_t index = 0,
        const std::string& account = std::string(""));
    bool Read(const std::string& id, const bool unread);
    bool Rename(const std::string& newID);
    bool Remove(const std::string& id);
    bool SetAlias(const std::string& alias);

    ~Thread() final = default;
};
}  // namespace storage
}  // namespace opentxs
