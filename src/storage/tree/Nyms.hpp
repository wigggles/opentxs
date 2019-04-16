// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/Types.hpp"

#include "Node.hpp"

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <tuple>

namespace opentxs
{
namespace storage
{
class Nyms : public Node
{
private:
    friend class Tree;

    mutable std::map<std::string, std::unique_ptr<storage::Nym>> nyms_;
    std::set<std::string> local_nyms_{};

    storage::Nym* nym(const std::string& id) const;
    storage::Nym* nym(const Lock& lock, const std::string& id) const;
    void save(storage::Nym* nym, const Lock& lock, const std::string& id);

    void init(const std::string& hash) override;
    bool save(const Lock& lock) const override;
    proto::StorageNymList serialize() const;

    Nyms(const opentxs::api::storage::Driver& storage, const std::string& hash);
    Nyms() = delete;
    Nyms(const Nyms&) = delete;
    Nyms(Nyms&&) = delete;
    Nyms operator=(const Nyms&) = delete;
    Nyms operator=(Nyms&&) = delete;

public:
    bool Exists(const std::string& id) const;
    const std::set<std::string> LocalNyms() const;
    void Map(NymLambda lambda) const;
    bool Migrate(const opentxs::api::storage::Driver& to) const override;
    const storage::Nym& Nym(const std::string& id) const;

    Editor<storage::Nym> mutable_Nym(const std::string& id);
    bool RelabelThread(const std::string& threadID, const std::string label);
    void UpgradeLocalnym();

    ~Nyms() = default;
};
}  // namespace storage
}  // namespace opentxs
