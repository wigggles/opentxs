// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/basket/BasketContract.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/identity/Nym.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace opentxs
{
BasketContract::BasketContract(
    const api::Core& api,
    const Nym_p& nym,
    const proto::UnitDefinition serialized)
    : ot_super(api, nym, serialized)
    , subcontracts_()
    , weight_(0)
{
    if (serialized.has_basket()) {

        if (serialized.basket().has_weight()) {
            weight_ = serialized.basket().weight();
        }

        for (auto& item : serialized.basket().item()) {
            subcontracts_.insert(
                {item.unit(), {item.account(), item.weight()}});
        }
    }
}

BasketContract::BasketContract(
    const api::Core& api,
    const Nym_p& nym,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms,
    const std::uint64_t weight,
    const proto::ContactItemType unitOfAccount,
    const VersionNumber version)
    : ot_super(api, nym, shortname, name, symbol, terms, unitOfAccount, version)
    , subcontracts_()
    , weight_(weight)
{
}

OTIdentifier BasketContract::CalculateBasketID(
    const api::Core& api,
    const proto::UnitDefinition& serialized)
{
    auto contract(serialized);
    contract.clear_id();
    contract.clear_nymid();
    contract.clear_publicnym();

    for (auto& item : *contract.mutable_basket()->mutable_item()) {
        item.clear_account();
    }

    return GetID(api, contract);
}

bool BasketContract::FinalizeTemplate(
    const api::Core& api,
    const Nym_p& nym,
    proto::UnitDefinition& serialized,
    const PasswordPrompt& reason)
{
    std::unique_ptr<BasketContract> contract(
        new BasketContract(api, nym, serialized));

    if (!contract) { return false; }

    Lock lock(contract->lock_);

    if (!contract->CalculateID(lock)) { return false; }

    if (contract->nym_) {
        proto::UnitDefinition basket = contract->SigVersion(lock);
        std::shared_ptr<proto::Signature> sig =
            std::make_shared<proto::Signature>();
        if (contract->update_signature(lock, reason)) {
            lock.unlock();
            serialized = contract->PublicContract();

            return proto::Validate(serialized, VERBOSE, false);
        }
    }

    return false;
}

proto::UnitDefinition BasketContract::IDVersion(const Lock& lock) const
{
    proto::UnitDefinition contract = ot_super::IDVersion(lock);

    contract.set_type(proto::UNITTYPE_BASKET);

    auto basket = contract.mutable_basket();
    basket->set_version(1);
    basket->set_weight(weight_);

    // determinism here depends on the defined ordering of std::map
    for (auto& item : subcontracts_) {
        auto serialized = basket->add_item();
        serialized->set_version(1);
        serialized->set_unit(item.first);
        serialized->set_account(item.second.first);
        serialized->set_weight(item.second.second);
    }

    return contract;
}

proto::UnitDefinition BasketContract::BasketIDVersion(const Lock& lock) const
{
    auto contract = ot_super::SigVersion(lock);

    for (auto& item : *(contract.mutable_basket()->mutable_item())) {
        item.clear_account();
    }

    return contract;
}
}  // namespace opentxs
