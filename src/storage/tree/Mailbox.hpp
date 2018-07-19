// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_STORAGE_TREE_MAILBOX_HPP
#define OPENTXS_STORAGE_TREE_MAILBOX_HPP

#include "Internal.hpp"

#include "opentxs/api/Editor.hpp"

#include "Node.hpp"

namespace opentxs
{
namespace storage
{

class Nym;

class Mailbox : public Node
{
private:
    friend class Nym;

    void init(const std::string& hash) override;
    bool save(const std::unique_lock<std::mutex>& lock) const override;
    proto::StorageNymList serialize() const;

    Mailbox(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash);
    Mailbox() = delete;
    Mailbox(const Mailbox&) = delete;
    Mailbox(Mailbox&&) = delete;
    Mailbox operator=(const Mailbox&) = delete;
    Mailbox operator=(Mailbox&&) = delete;

public:
    bool Load(
        const std::string& id,
        std::string& output,
        std::string& alias,
        const bool checking) const;

    bool Delete(const std::string& id);
    bool Store(
        const std::string& id,
        const std::string& data,
        const std::string& alias);

    ~Mailbox() = default;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_MAILBOX_HPP
