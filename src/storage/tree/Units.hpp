// Copyright (c) 2010-2019 The Open-Transactions developers
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
class Units final : public Node
{
private:
    friend class Tree;

    void init(const std::string& hash) final;
    bool save(const std::unique_lock<std::mutex>& lock) const final;
    proto::StorageUnits serialize() const;

    Units(const opentxs::api::storage::Driver& storage, const std::string& key);
    Units() = delete;
    Units(const Units&) = delete;
    Units(Units&&) = delete;
    Units operator=(const Units&) = delete;
    Units operator=(Units&&) = delete;

public:
    std::string Alias(const std::string& id) const;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::UnitDefinition>& output,
        std::string& alias,
        const bool checking) const;
    void Map(UnitLambda lambda) const;

    bool Delete(const std::string& id);
    bool SetAlias(const std::string& id, const std::string& alias);
    bool Store(
        const proto::UnitDefinition& data,
        const std::string& alias,
        std::string& plaintext);

    ~Units() final = default;
};
}  // namespace storage
}  // namespace opentxs
