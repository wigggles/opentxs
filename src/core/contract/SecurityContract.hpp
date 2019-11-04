// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::contract::unit::implementation
{
class Security final : public unit::Security,
                       public contract::implementation::Unit
{
public:
    std::string TLA() const final { return primary_unit_symbol_; }
    proto::UnitType Type() const final { return proto::UNITTYPE_SECURITY; }

    Security(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version);
    Security(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized);

    ~Security() final = default;

private:
    friend opentxs::Factory;

    Security* clone() const noexcept final { return new Security(*this); }
    proto::UnitDefinition IDVersion(const Lock& lock) const final;

    Security(const Security&);
    Security(Security&&) = delete;
    Security& operator=(const Security&) = delete;
    Security& operator=(Security&&) = delete;
};
}  // namespace opentxs::contract::unit::implementation
