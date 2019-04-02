// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_IDENTIFIER_UNITDEFINITION_HPP
#define OPENTXS_CORE_IDENTIFIER_UNITDEFINITION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Identifier.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::identifier::UnitDefinition::Factory;
%extend opentxs::identifier::UnitDefinition {
    static OTUnitID Factory()
    {
        return opentxs::identifier::UnitDefinition::Factory();
    }
    static OTUnitID Factory(
        const std::string& rhs)
    {
        return opentxs::identifier::UnitDefinition::Factory(rhs);
    }
}
%rename (UnitDefinitionID) opentxs::identifier::UnitDefinition;
%template(OTUnitID) opentxs::Pimpl<opentxs::identifier::UnitDefinition>;
// clang-format on
#endif

namespace opentxs
{
#ifndef SWIG
bool operator==(
    const opentxs::Pimpl<opentxs::identifier::UnitDefinition>& lhs,
    const opentxs::Identifier& rhs);
bool operator!=(
    const opentxs::Pimpl<opentxs::identifier::UnitDefinition>& lhs,
    const opentxs::Identifier& rhs);
bool operator<(
    const opentxs::Pimpl<opentxs::identifier::UnitDefinition>& lhs,
    const opentxs::Pimpl<opentxs::identifier::UnitDefinition>& rhs);
#endif

namespace identifier
{
class UnitDefinition : virtual public opentxs::Identifier
{
public:
#ifndef SWIG
    EXPORT static OTUnitID Factory();
    EXPORT static OTUnitID Factory(const std::string& rhs);
    EXPORT static OTUnitID Factory(const String& rhs);
#endif

    EXPORT virtual ~UnitDefinition() = default;

protected:
    UnitDefinition() = default;

private:
    friend OTUnitID;

    virtual UnitDefinition* clone() const = 0;

    UnitDefinition(const UnitDefinition&) = delete;
    UnitDefinition(UnitDefinition&&) = delete;
    UnitDefinition& operator=(const UnitDefinition&) = delete;
    UnitDefinition& operator=(UnitDefinition&&) = delete;
};
}  // namespace identifier
}  // namespace opentxs
#endif
