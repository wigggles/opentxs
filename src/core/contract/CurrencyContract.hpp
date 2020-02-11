// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::contract::unit::implementation
{
class Currency final : public unit::Currency,
                       public contract::implementation::Unit
{
public:
    std::int32_t DecimalPower() const final { return power_; }
    std::string FractionalUnitName() const final
    {
        return fractional_unit_name_;
    }
    std::string TLA() const final { return tla_; }
    proto::UnitType Type() const final { return proto::UNITTYPE_CURRENCY; }

    Currency(
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
    Currency(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized);

    ~Currency() final = default;

private:
    // ISO-4217. E.g., USD, AUG, PSE. Take as hint, not as contract.
    const std::string tla_;
    // If value is 103, decimal power of 0 displays 103 (actual value.) Whereas
    // decimal power of 2 displays 1.03 and 4 displays .0103
    const std::uint32_t power_;
    // "cents"
    const std::string fractional_unit_name_;

    Currency* clone() const noexcept final { return new Currency(*this); }
    proto::UnitDefinition IDVersion(const Lock& lock) const final;

    Currency(const Currency&);
    Currency(Currency&&) = delete;
    Currency& operator=(const Currency&) = delete;
    Currency& operator=(Currency&&) = delete;
};
}  // namespace opentxs::contract::unit::implementation
