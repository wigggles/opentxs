// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/Editor.hpp"

#include "Node.hpp"

#include <memory>
#include <string>

namespace opentxs
{
namespace storage
{
class Credentials final : public Node
{
private:
    friend class Tree;

    bool check_existing(const bool incoming, Metadata& metadata) const;
    void init(const std::string& hash) final;
    bool save(const std::unique_lock<std::mutex>& lock) const final;
    proto::StorageCredentials serialize() const;

    Credentials(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash);
    Credentials() = delete;
    Credentials(const Credentials&) = delete;
    Credentials(Credentials&&) = delete;
    Credentials operator=(const Credentials&) = delete;
    Credentials operator=(Credentials&&) = delete;

public:
    std::string Alias(const std::string& id) const;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Credential>& output,
        const bool checking) const;

    bool Delete(const std::string& id);
    bool SetAlias(const std::string& id, const std::string& alias);
    bool Store(const proto::Credential& data, const std::string& alias);

    ~Credentials() final = default;
};
}  // namespace storage
}  // namespace opentxs
