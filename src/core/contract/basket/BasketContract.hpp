// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::contract::unit::implementation
{
class Basket final : public unit::Basket, public contract::implementation::Unit
{
public:
    OTIdentifier BasketID() const final;
    const Subcontracts& Currencies() const final { return subcontracts_; }
    proto::UnitType Type() const final { return proto::UNITTYPE_BASKET; }
    std::uint64_t Weight() const final { return weight_; }

    Basket(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::uint64_t weight,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version);
    Basket(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized);

    ~Basket() final = default;

private:
    friend unit::Basket;

    Subcontracts subcontracts_;
    std::uint64_t weight_;

    proto::UnitDefinition BasketIDVersion(const Lock& lock) const;
    Basket* clone() const noexcept final { return new Basket(*this); }
    proto::UnitDefinition IDVersion(const Lock& lock) const final;

    Basket(const Basket&);
    Basket(Basket&&) = delete;
    Basket& operator=(const Basket&) = delete;
    Basket& operator=(Basket&&) = delete;
};
}  // namespace opentxs::contract::unit::implementation
