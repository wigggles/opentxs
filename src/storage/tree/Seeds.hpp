// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/Editor.hpp"

#include "Node.hpp"

#include <cstdint>

namespace opentxs
{
namespace storage
{
class Seeds final : public Node
{
private:
    friend class Tree;

    std::string default_seed_;

    void init(const std::string& hash) final;
    bool save(const std::unique_lock<std::mutex>& lock) const final;
    void set_default(
        const std::unique_lock<std::mutex>& lock,
        const std::string& id);
    proto::StorageSeeds serialize() const;

    Seeds(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash);
    Seeds() = delete;
    Seeds(const Seeds&) = delete;
    Seeds(Seeds&&) = delete;
    Seeds operator=(const Seeds&) = delete;
    Seeds operator=(Seeds&&) = delete;

public:
    std::string Alias(const std::string& id) const;
    std::string Default() const;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Seed>& output,
        std::string& alias,
        const bool checking) const;

    bool Delete(const std::string& id);
    bool SetAlias(const std::string& id, const std::string& alias);
    bool SetDefault(const std::string& id);
    bool Store(const proto::Seed& data, const std::string& alias);

    ~Seeds() final = default;
};
}  // namespace storage
}  // namespace opentxs
