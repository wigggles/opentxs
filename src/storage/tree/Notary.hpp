// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Editor.hpp"

#include "Node.hpp"

#include <map>

namespace opentxs::storage
{
class Notary final : public Node
{
public:
    using MintSeries = std::uint64_t;

#if OT_CASH
    bool CheckSpent(
        const identifier::UnitDefinition& unit,
        const MintSeries series,
        const std::string& key) const;

    bool MarkSpent(
        const identifier::UnitDefinition& unit,
        const MintSeries series,
        const std::string& key);
#endif

    ~Notary() final = default;

private:
    friend class Tree;
    using SeriesMap = std::map<MintSeries, std::string>;
    using UnitMap = std::map<std::string, SeriesMap>;

    std::string id_;

#if OT_CASH
    mutable UnitMap mint_map_;

    std::string create_list(
        const std::string& unitID,
        const MintSeries series,
        std::shared_ptr<proto::SpentTokenList>& output) const;
    proto::SpentTokenList get_or_create_list(
        const Lock& lock,
        const std::string& unitID,
        const MintSeries series) const;
#endif
    bool save(const Lock& lock) const final;
    proto::StorageNotary serialize() const;

    void init(const std::string& hash) final;

    Notary(
        const opentxs::api::storage::Driver& storage,
        const std::string& key,
        const std::string& id);
    Notary() = delete;
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    Notary operator=(const Notary&) = delete;
    Notary operator=(Notary&&) = delete;
};
}  // namespace opentxs::storage
