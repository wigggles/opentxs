// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Editor.hpp"

#include "Node.hpp"

namespace opentxs
{
namespace storage
{
class Servers final : public Node
{
private:
    friend class Tree;

    void init(const std::string& hash) final;
    bool save(const std::unique_lock<std::mutex>& lock) const final;
    proto::StorageServers serialize() const;

    Servers(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash);
    Servers() = delete;
    Servers(const Servers&) = delete;
    Servers(Servers&&) = delete;
    Servers operator=(const Servers&) = delete;
    Servers operator=(Servers&&) = delete;

public:
    std::string Alias(const std::string& id) const;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::ServerContract>& output,
        std::string& alias,
        const bool checking) const;
    void Map(ServerLambda lambda) const;

    bool Delete(const std::string& id);
    bool SetAlias(const std::string& id, const std::string& alias);
    bool Store(
        const proto::ServerContract& data,
        const std::string& alias,
        std::string& plaintext);

    ~Servers() final = default;
};
}  // namespace storage
}  // namespace opentxs
