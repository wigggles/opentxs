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

#include "opentxs/core/Identifier.hpp"

#include "opentxs/core/app/App.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHash.hpp"
#include "opentxs/core/crypto/CryptoUtil.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTSymmetricKey.hpp"
#include "opentxs/core/stdafx.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{

// static
bool Identifier::validateID(const std::string & strPurportedID)
{
    if (strPurportedID.empty()) {
        return false;
    }
    Identifier theID(strPurportedID);

    return (0 < theID.GetSize());
}

proto::HashType Identifier::IDToHashType(const ID type)
{
    switch (type) {
        case (ID::SHA256) : { return proto::HASHTYPE_SHA256; }
        case (ID::BTC160) : { return proto::HASHTYPE_BTC160; }
        default : { return proto::HASHTYPE_NONE; }
    }
}

Identifier::Identifier()
    : OTData()
{
}

Identifier::Identifier(const Identifier& theID)
    : OTData(theID)
    , type_(theID.Type())
{
}

Identifier::Identifier(const std::string& theStr)
    : OTData()
{
    SetString(theStr);
}

Identifier::Identifier(const String& theStr)
    : OTData()
{
    SetString(theStr);
}

Identifier::Identifier(const Contract& theContract)
    : OTData() // Get the contract's ID into this identifier.
{
    (const_cast<Contract&>(theContract)).GetIdentifier(*this);
}

Identifier::Identifier(const Nym& theNym)
    : OTData() // Get the Nym's ID into this identifier.
{
    (const_cast<Nym&>(theNym)).GetIdentifier(*this);
}

Identifier::Identifier(const OTSymmetricKey& theKey)
    : OTData() // Get the Symmetric Key's ID into *this. (It's a hash of the
               // encrypted form of the symmetric key.)
{
    (const_cast<OTSymmetricKey&>(theKey)).GetIdentifier(*this);
}

Identifier::Identifier(const OTCachedKey& theKey)
    : OTData() // Cached Key stores a symmetric key inside, so this actually
               // captures the ID for that symmetrickey.
{
    const bool bSuccess =
        (const_cast<OTCachedKey&>(theKey)).GetIdentifier(*this);

    OT_ASSERT(bSuccess); // should never fail. If it does, then we are calling
                         // this function at a time we shouldn't, when we aren't
                         // sure the master key has even been generated yet. (If
                         // this asserts, need to examine the line of code that
                         // tried to do this, and figure out where its logic
                         // went wrong, since it should have made sure this
                         // would not happen, before constructing like this.)
}

Identifier& Identifier::operator=(Identifier rhs)
{
    swap(rhs);
    return *this;
}

bool Identifier::operator==(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.Compare(ots2);
}

bool Identifier::operator!=(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return !(ots1.Compare(ots2));
}

bool Identifier::operator>(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.operator>(ots2);
}

bool Identifier::operator<(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.operator<(ots2);
}

bool Identifier::operator<=(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.operator<=(ots2);
}

bool Identifier::operator>=(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.operator>=(ots2);
}

bool Identifier::CalculateDigest(const String& strInput, const ID type)
{
    type_ = type;

    return App::Me().Crypto().Hash().Digest(
        IDToHashType(type_),
        strInput,
        *this);
}

bool Identifier::CalculateDigest(const OTData& dataInput, const ID type)
{
    type_ = type;

    return App::Me().Crypto().Hash().Digest(
        IDToHashType(type_),
        dataInput,
        *this);
}

// SET (binary id) FROM ENCODED STRING
void Identifier::SetString(const String& encoded)
{
    return SetString(std::string(encoded.Get()));
}

void Identifier::SetString(const std::string& encoded)
{
    empty();

    if (MinimumSize > encoded.size()) { return; }

    if ('o' != encoded.at(0)) { return; }
    if ('t' != encoded.at(1)) { return; }

    OTData data;
    const bool decoded = App::Me().Crypto().Util().Base58CheckDecode(
        String(&encoded.at(2), (encoded.size() - 2)), data);

    if (decoded) {
        OTPassword::safe_memcpy(&type_, 1, data.GetPointer(), 1);

        switch (type_) {
            case (ID::SHA256) : { break; }
            case (ID::BTC160) : { break; }
            default : {
                type_ = ID::ERROR;

                return;
            }
        }

        Assign(
            (static_cast<const uint8_t*>(data.GetPointer()) + 1),
            (data.GetSize() - 1));
    }
}

// This Identifier is stored in binary form.
// But what if you want a pretty string version of it?
// Just call this function.
void Identifier::GetString(String& id) const
{
    OTData data;
    data.Assign(&type_, sizeof(type_));

    OT_ASSERT(1 == data.GetSize());

    data.Concatenate(GetPointer(), GetSize());

    String output("ot");
    output.Concatenate(
        String(App::Me().Crypto().Util().Base58CheckEncode(data)));
    id.swap(output);
}
} // namespace opentxs
