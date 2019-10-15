// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_CURRENCYCONTRACT_HPP
#define OPENTXS_CORE_CONTRACT_CURRENCYCONTRACT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/UnitDefinition.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

class CurrencyContract final : public UnitDefinition
{
private:
    typedef UnitDefinition ot_super;
    friend ot_super;

    // ISO-4217. E.g., USD, AUG, PSE. Take as hint, not as contract.
    std::string tla_;
    // If value is 103, decimal power of 0 displays 103 (actual value.) Whereas
    // decimal power of 2 displays 1.03 and 4 displays .0103
    std::uint32_t power_{};
    // "cents"
    std::string fractional_unit_name_;

    EXPORT CurrencyContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized);
    EXPORT CurrencyContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::string& tla,
        const std::uint32_t power,
        const std::string& fraction,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version);

    EXPORT proto::UnitDefinition IDVersion(const Lock& lock) const final;

public:
    EXPORT proto::UnitType Type() const final
    {
        return proto::UNITTYPE_CURRENCY;
    }

    EXPORT std::int32_t DecimalPower() const final { return power_; }
    EXPORT std::string FractionalUnitName() const final
    {
        return fractional_unit_name_;
    }  // "cents"    (for example)
    EXPORT std::string TLA() const final
    {
        return tla_;
    }  // "USD""     (for example)

    ~CurrencyContract() final = default;
};
}  // namespace opentxs
#endif
