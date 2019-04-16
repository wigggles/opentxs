// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/Editor.hpp"

#include "Node.hpp"

namespace opentxs
{
namespace storage
{
class Contexts : public Node
{
private:
    friend Nym;

    void init(const std::string& hash) override;
    bool save(const std::unique_lock<std::mutex>& lock) const override;
    proto::StorageNymList serialize() const;

    Contexts(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash);
    Contexts() = delete;
    Contexts(const Contexts&) = delete;
    Contexts(Contexts&&) = delete;
    Contexts operator=(const Contexts&) = delete;
    Contexts operator=(Contexts&&) = delete;

public:
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Context>& output,
        std::string& alias,
        const bool checking) const;

    bool Delete(const std::string& id);
    bool Store(const proto::Context& data, const std::string& alias);

    ~Contexts() = default;
};
}  // namespace storage
}  // namespace opentxs
