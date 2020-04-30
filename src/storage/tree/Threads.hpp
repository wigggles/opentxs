// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <tuple>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "storage/tree/Node.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
class Driver;
}  // namespace storage
}  // namespace api

namespace storage
{
class Mailbox;
class Nym;

class Threads final : public Node
{
private:
    typedef Node ot_super;
    friend Nym;

    mutable std::map<std::string, std::unique_ptr<class Thread>> threads_;
    Mailbox& mail_inbox_;
    Mailbox& mail_outbox_;

    bool save(const std::unique_lock<std::mutex>& lock) const final;
    proto::StorageNymList serialize() const;
    class Thread* thread(const std::string& id) const;
    class Thread* thread(
        const std::string& id,
        const std::unique_lock<std::mutex>& lock) const;

    std::string create(
        const Lock& lock,
        const std::string& id,
        const std::set<std::string>& participants);
    void init(const std::string& hash) final;
    void save(
        class Thread* thread,
        const std::unique_lock<std::mutex>& lock,
        const std::string& id);

    Threads(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash,
        Mailbox& mailInbox,
        Mailbox& mailOutbox);
    Threads() = delete;
    Threads(const Threads&) = delete;
    Threads(Threads&&) = delete;
    Threads operator=(const Threads&) = delete;
    Threads operator=(Threads&&) = delete;

public:
    bool Exists(const std::string& id) const;
    using ot_super::List;
    ObjectList List(const bool unreadOnly) const;
    bool Migrate(const opentxs::api::storage::Driver& to) const final;
    const class Thread& Thread(const std::string& id) const;

    std::string Create(
        const std::string& id,
        const std::set<std::string>& participants);
    bool FindAndDeleteItem(const std::string& itemID);
    Editor<class Thread> mutable_Thread(const std::string& id);
    bool Rename(const std::string& existingID, const std::string& newID);

    ~Threads() final = default;
};
}  // namespace storage
}  // namespace opentxs
