// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_IDENTIFIER_NYM_HPP
#define OPENTXS_CORE_IDENTIFIER_NYM_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Identifier.hpp"

#ifdef SWIG
// clang-format off
%rename (NymIDFactory) opentxs::identifier::Nym::Factory;
%template(OTNymID) opentxs::Pimpl<opentxs::identifier::Nym>;
// clang-format on
#endif

namespace opentxs
{
namespace identifier
{
class Nym : virtual public Identifier
{
public:
    EXPORT static opentxs::Pimpl<opentxs::identifier::Nym> Factory();
    EXPORT static opentxs::Pimpl<opentxs::identifier::Nym> Factory(
        const std::string& rhs);
#ifndef SWIG
    EXPORT static opentxs::Pimpl<opentxs::identifier::Nym> Factory(
        const String& rhs);
    EXPORT static opentxs::Pimpl<opentxs::identifier::Nym> Factory(
        const opentxs::Nym& nym);
#endif

    EXPORT virtual ~Nym() = default;

protected:
    Nym() = default;

private:
    friend opentxs::Pimpl<opentxs::identifier::Nym>;

    virtual Nym* clone() const = 0;

    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    Nym& operator=(const Nym&) = delete;
    Nym& operator=(Nym&&) = delete;
};
}  // namespace identifier
}  // namespace opentxs
#endif
