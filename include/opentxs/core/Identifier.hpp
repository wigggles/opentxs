// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_IDENTIFIER_HPP
#define OPENTXS_CORE_IDENTIFIER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::Identifier>::Pimpl(opentxs::Identifier const &);
%ignore opentxs::Pimpl<opentxs::Identifier>::operator opentxs::Identifier&;
%ignore opentxs::Pimpl<opentxs::Identifier>::operator const opentxs::Identifier &;
%rename(identifierCompareEqual) opentxs::Identifier::operator==(const Identifier& rhs) const;
%rename(identifierCompareNotEqual) opentxs::Identifier::operator!=(const Identifier& rhs) const;
%rename(identifierCompareGreaterThan) opentxs::Identifier::operator>(const Identifier& rhs) const;
%rename(identifierCompareLessThan) opentxs::Identifier::operator<(const Identifier& rhs) const;
%rename(identifierCompareGreaterOrEqual) opentxs::Identifier::operator>=(const Identifier& rhs) const;
%rename(identifierCompareLessOrEqual) opentxs::Identifier::operator<=(const Identifier& rhs) const;
%rename (IdentifierFactory) opentxs::Identifier::Factory;
%template(OTIdentifier) opentxs::Pimpl<opentxs::Identifier>;
// clang-format on
#endif

namespace opentxs
{
#ifndef SWIG
bool operator==(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs);
bool operator!=(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs);
bool operator<(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs);
bool operator>(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs);
bool operator<=(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs);
bool operator>=(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs);
#endif

/** An Identifier is basically a 256 bit hash value. This class makes it easy to
 * convert IDs back and forth to strings. */
class Identifier : virtual public Data
{
public:
    using ot_super = opentxs::Data;

    EXPORT static opentxs::Pimpl<opentxs::Identifier> Random();
    EXPORT static opentxs::Pimpl<opentxs::Identifier> Factory();
    EXPORT static opentxs::Pimpl<opentxs::Identifier> Factory(
        const Identifier& rhs);
    EXPORT static opentxs::Pimpl<opentxs::Identifier> Factory(
        const std::string& rhs);
#ifndef SWIG
    EXPORT static opentxs::Pimpl<opentxs::Identifier> Factory(
        const String& rhs);
    EXPORT static opentxs::Pimpl<opentxs::Identifier> Factory(
        const identity::Nym& nym);
    EXPORT static opentxs::Pimpl<opentxs::Identifier> Factory(
        const Cheque& cheque);
    EXPORT static opentxs::Pimpl<opentxs::Identifier> Factory(const Item& item);
    EXPORT static opentxs::Pimpl<opentxs::Identifier> Factory(
        const Contract& contract);
    EXPORT static opentxs::Pimpl<opentxs::Identifier> Factory(
        const crypto::key::LegacySymmetric& key);
    EXPORT static opentxs::Pimpl<opentxs::Identifier> Factory(
        const OTCachedKey& key);
    EXPORT static opentxs::Pimpl<opentxs::Identifier> Factory(
        const proto::ContactItemType type,
        const proto::HDPath& path);
#endif
    EXPORT static bool Validate(const std::string& id);

    using ot_super::operator==;
    EXPORT virtual bool operator==(const Identifier& rhs) const = 0;
    using ot_super::operator!=;
    EXPORT virtual bool operator!=(const Identifier& rhs) const = 0;
    using ot_super::operator>;
    EXPORT virtual bool operator>(const Identifier& rhs) const = 0;
    using ot_super::operator<;
    EXPORT virtual bool operator<(const Identifier& rhs) const = 0;
    using ot_super::operator<=;
    EXPORT virtual bool operator<=(const Identifier& rhs) const = 0;
    using ot_super::operator>=;
    EXPORT virtual bool operator>=(const Identifier& rhs) const = 0;

#ifndef SWIG
    EXPORT virtual void GetString(String& theStr) const = 0;
#endif
    EXPORT virtual std::string str() const = 0;
    EXPORT virtual const ID& Type() const = 0;

    EXPORT virtual bool CalculateDigest(
        const Data& input,
        const ID type = ID::BLAKE2B) = 0;
#ifndef SWIG
    EXPORT virtual bool CalculateDigest(
        const String& input,
        const ID type = ID::BLAKE2B) = 0;
#endif
    EXPORT virtual void SetString(const std::string& encoded) = 0;
#ifndef SWIG
    EXPORT virtual void SetString(const String& encoded) = 0;
#endif
    using ot_super::swap;
    EXPORT virtual void swap(Identifier& rhs) = 0;

    EXPORT virtual ~Identifier() = default;

protected:
    Identifier() = default;

private:
    friend opentxs::Pimpl<opentxs::Identifier>;

    virtual Identifier* clone() const = 0;

    Identifier(const Identifier&) = delete;
    Identifier(Identifier&&) = delete;
    Identifier& operator=(const Identifier&) = delete;
    Identifier& operator=(Identifier&&) = delete;
};
}  // namespace opentxs
#endif
