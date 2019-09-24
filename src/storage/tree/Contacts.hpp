// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/Editor.hpp"

#include "Node.hpp"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <tuple>

namespace opentxs::storage
{
class Contacts final : public Node
{
public:
    std::string Alias(const std::string& id) const;
    std::string AddressOwner(proto::ContactItemType chain, std::string address)
        const;
    ObjectList List() const final;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Contact>& output,
        std::string& alias,
        const bool checking) const;
    std::string NymOwner(std::string nym) const;
    bool Save() const;

    bool Delete(const std::string& id);
    bool SetAlias(const std::string& id, const std::string& alias);
    bool Store(
        const proto::Contact& data,
        const std::string& alias,
        std::map<OTData, OTIdentifier>& changed);

    ~Contacts() final = default;

private:
    friend class Tree;
    typedef Node ot_super;
    typedef std::pair<proto::ContactItemType, std::string> Address;

    static const VersionNumber CurrentVersion{2};
    static const VersionNumber AddressIndexVersion{1};
    static const VersionNumber MergeIndexVersion{1};
    static const VersionNumber NymIndexVersion{1};

    mutable std::map<Address, std::string> address_index_;
    mutable std::map<std::string, std::set<Address>> address_reverse_index_;
    std::map<std::string, std::set<std::string>> merge_;
    std::map<std::string, std::string> merged_;
    mutable std::map<std::string, std::string> nym_contact_index_;

    void extract_addresses(
        const Lock& lock,
        const proto::Contact& data,
        std::map<OTData, OTIdentifier>& changed) const;
    void extract_nyms(const Lock& lock, const proto::Contact& data) const;
    const std::string& nomalize_id(const std::string& input) const;
    bool save(const std::unique_lock<std::mutex>& lock) const final;
    proto::StorageContacts serialize() const;

    void init(const std::string& hash) final;
    void reconcile_maps(const Lock& lock, const proto::Contact& data);
    void reverse_merged();

    Contacts(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash);
    Contacts() = delete;
    Contacts(const Contacts&) = delete;
    Contacts(Contacts&&) = delete;
    Contacts operator=(const Contacts&) = delete;
    Contacts operator=(Contacts&&) = delete;
};
}  // namespace opentxs::storage
