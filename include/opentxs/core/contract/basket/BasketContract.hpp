// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_BASKET_BASKETCONTRACT_HPP
#define OPENTXS_CORE_CONTRACT_BASKET_BASKETCONTRACT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/UnitDefinition.hpp"

namespace opentxs
{
class BasketContract final : public UnitDefinition
{
private:
    typedef UnitDefinition ot_super;
    // account number, weight
    typedef std::pair<std::string, std::uint64_t> Subcontract;
    // unit definition id, subcontract
    typedef std::map<std::string, Subcontract> MapOfSubcontracts;
    friend ot_super;
    friend UserCommandProcessor;

    MapOfSubcontracts subcontracts_;
    std::uint64_t weight_;

    EXPORT BasketContract(
        const api::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized);
    EXPORT BasketContract(
        const api::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::uint64_t weight,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version);

    EXPORT proto::UnitDefinition BasketIDVersion(const Lock& lock) const;
    EXPORT proto::UnitDefinition IDVersion(const Lock& lock) const final;

public:
    EXPORT static OTIdentifier CalculateBasketID(
        const api::Core& api,
        const proto::UnitDefinition& serialized);
    EXPORT static bool FinalizeTemplate(
        const api::Core& api,
        const Nym_p& nym,
        proto::UnitDefinition& serialized,
        const PasswordPrompt& reason);

    EXPORT OTIdentifier BasketID() const;
    EXPORT const MapOfSubcontracts& Currencies() const { return subcontracts_; }
    EXPORT proto::UnitType Type() const final { return proto::UNITTYPE_BASKET; }
    EXPORT std::uint64_t Weight() const { return weight_; }

    ~BasketContract() final = default;
};
}  // namespace opentxs
#endif
