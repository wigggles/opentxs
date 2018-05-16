/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CORE_OTIDENTIFIER_HPP
#define OPENTXS_CORE_OTIDENTIFIER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Data_imp.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <iosfwd>
#include <string>
#include <set>
#include <map>

namespace opentxs
{
bool operator==(
    const opentxs::OTIdentifier& lhs,
    const opentxs::Identifier& rhs);
bool operator!=(
    const opentxs::OTIdentifier& lhs,
    const opentxs::Identifier& rhs);
bool operator<(
    const opentxs::OTIdentifier& lhs,
    const opentxs::OTIdentifier& rhs);

/** An Identifier is basically a 256 bit hash value. This class makes it easy to
 * convert IDs back and forth to strings. */
class Identifier : virtual public implementation::Data
{
private:
    using ot_super = opentxs::implementation::Data;

public:
    EXPORT static OTIdentifier Random();
    EXPORT static OTIdentifier Factory();
    EXPORT static OTIdentifier Factory(const Identifier& rhs);
    EXPORT static OTIdentifier Factory(const std::string& rhs);
    EXPORT static OTIdentifier Factory(const String& rhs);
    EXPORT static OTIdentifier Factory(const Nym& nym);
    EXPORT static OTIdentifier Factory(const Contract& contract);
    EXPORT static OTIdentifier Factory(const Cheque& cheque);
    EXPORT static OTIdentifier Factory(const OTSymmetricKey& key);
    EXPORT static OTIdentifier Factory(const OTCachedKey& key);
    EXPORT static OTIdentifier Factory(
        const proto::ContactItemType type,
        const proto::HDPath& path);

    EXPORT friend std::ostream& operator<<(std::ostream& os, const String& obj);
    EXPORT static bool validateID(const std::string& strPurportedID);

    using ot_super::operator==;
    EXPORT bool operator==(const Identifier& s2) const;
    using ot_super::operator!=;
    EXPORT bool operator!=(const Identifier& s2) const;
    EXPORT bool operator>(const Identifier& s2) const;
    EXPORT bool operator<(const Identifier& s2) const;
    EXPORT bool operator<=(const Identifier& s2) const;
    EXPORT bool operator>=(const Identifier& s2) const;

    EXPORT void GetString(String& theStr) const;
    EXPORT std::string str() const;

    EXPORT bool CalculateDigest(
        const opentxs::Data& dataInput,
        const ID type = DefaultType);
    EXPORT bool CalculateDigest(
        const String& strInput,
        const ID type = DefaultType);
    /** If someone passes in the pretty string of alphanumeric digits, convert
     * it to the actual binary hash and set it internally. */
    EXPORT void SetString(const std::string& encoded);
    EXPORT void SetString(const String& encoded);
    using ot_super::swap;
    void swap(Identifier&& rhs);
    /** theStr will contain pretty hex string after call. */
    EXPORT const ID& Type() const { return type_; }

    EXPORT virtual ~Identifier() = default;

private:
    friend OTIdentifier;

    static const ID DefaultType{ID::BLAKE2B};
    static const size_t MinimumSize{10};

    ID type_{DefaultType};

    Identifier* clone() const;

    static proto::HashType IDToHashType(const ID type);
    static OTData path_to_data(
        const proto::ContactItemType type,
        const proto::HDPath& path);

public:
    Identifier();
    explicit Identifier(const std::string& rhs);
    explicit Identifier(const String& rhs);
    explicit Identifier(const Nym& nym);
    explicit Identifier(const Contract& contract);
    explicit Identifier(const OTSymmetricKey& key);
    explicit Identifier(const OTCachedKey& key);
    explicit Identifier(
        const proto::ContactItemType type,
        const proto::HDPath& path);
    Identifier(const Identifier& rhs);

    Identifier& operator=(const Identifier& rhs);
    Identifier& operator=(Identifier&& rhs);
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_OTIDENTIFIER_HPP
