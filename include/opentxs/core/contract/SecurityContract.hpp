// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_SECURITYCONTRACT_HPP
#define OPENTXS_CORE_CONTRACT_SECURITYCONTRACT_HPP

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

class SecurityContract final : public UnitDefinition
{
private:
    typedef UnitDefinition ot_super;
    friend ot_super;

    SecurityContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized);
    SecurityContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version);

    proto::UnitDefinition IDVersion(const Lock& lock) const final;

public:
    EXPORT proto::UnitType Type() const final
    {
        return proto::UNITTYPE_SECURITY;
    }

    EXPORT std::string TLA() const final { return primary_unit_symbol_; }

    ~SecurityContract() final = default;
};
}  // namespace opentxs
#endif
