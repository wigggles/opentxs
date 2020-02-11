// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

#include "Node.hpp"

#include <map>
#include <set>

namespace opentxs::storage
{
class Accounts final : public Node
{
public:
    OTUnitID AccountContract(const Identifier& accountID) const;
    OTNymID AccountIssuer(const Identifier& accountID) const;
    OTNymID AccountOwner(const Identifier& accountID) const;
    OTServerID AccountServer(const Identifier& accountID) const;
    OTNymID AccountSigner(const Identifier& accountID) const;
    proto::ContactItemType AccountUnit(const Identifier& accountID) const;
    std::set<OTIdentifier> AccountsByContract(
        const identifier::UnitDefinition& unit) const;
    std::set<OTIdentifier> AccountsByIssuer(
        const identifier::Nym& issuerNym) const;
    std::set<OTIdentifier> AccountsByOwner(
        const identifier::Nym& ownerNym) const;
    std::set<OTIdentifier> AccountsByServer(
        const identifier::Server& server) const;
    std::set<OTIdentifier> AccountsByUnit(
        const proto::ContactItemType unit) const;
    std::string Alias(const std::string& id) const;
    bool Load(
        const std::string& id,
        std::string& output,
        std::string& alias,
        const bool checking) const;

    bool Delete(const std::string& id);
    bool SetAlias(const std::string& id, const std::string& alias);
    bool Store(
        const std::string& id,
        const std::string& data,
        const std::string& alias,
        const identifier::Nym& ownerNym,
        const identifier::Nym& signerNym,
        const identifier::Nym& issuerNym,
        const identifier::Server& server,
        const identifier::UnitDefinition& contract,
        const proto::ContactItemType unit);

    ~Accounts() final = default;

private:
    friend class Tree;

    using NymIndex = std::map<OTNymID, std::set<OTIdentifier>>;
    using ServerIndex = std::map<OTServerID, std::set<OTIdentifier>>;
    using ContractIndex = std::map<OTUnitID, std::set<OTIdentifier>>;
    using UnitIndex = std::map<proto::ContactItemType, std::set<OTIdentifier>>;
    /** owner, signer, issuer, server, contract, unit */
    using AccountData = std::tuple<
        OTNymID,
        OTNymID,
        OTNymID,
        OTServerID,
        OTUnitID,
        proto::ContactItemType>;
    using ReverseIndex = std::map<OTIdentifier, AccountData>;

    NymIndex owner_index_{};
    NymIndex signer_index_{};
    NymIndex issuer_index_{};
    ServerIndex server_index_{};
    ContractIndex contract_index_{};
    UnitIndex unit_index_{};
    mutable ReverseIndex account_data_{};

    template <typename A, typename M, typename I>
    static bool add_set_index(
        const Identifier& accountID,
        const A& argID,
        M& mapID,
        I& index);

    template <typename K, typename I>
    static void erase(const Identifier& accountID, const K& key, I& index)
    {
        try {
            auto& set = index.at(key);
            set.erase(accountID);

            if (0 == set.size()) { index.erase(key); }
        } catch (...) {
        }
    }

    AccountData& get_account_data(
        const Lock& lock,
        const OTIdentifier& accountID) const;
    proto::StorageAccounts serialize() const;

    bool check_update_account(
        const Lock& lock,
        const OTIdentifier& accountID,
        const identifier::Nym& ownerNym,
        const identifier::Nym& signerNym,
        const identifier::Nym& issuerNym,
        const identifier::Server& server,
        const identifier::UnitDefinition& contract,
        const proto::ContactItemType unit);
    void init(const std::string& hash) final;
    bool save(const Lock& lock) const final;

    Accounts(
        const opentxs::api::storage::Driver& storage,
        const std::string& key);
    Accounts() = delete;
    Accounts(const Accounts&) = delete;
    Accounts(Accounts&&) = delete;
    Accounts operator=(const Accounts&) = delete;
    Accounts operator=(Accounts&&) = delete;
};
}  // namespace opentxs::storage
