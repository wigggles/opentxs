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

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/Identifier.hpp>
#include <opentxs/core/Contract.hpp>
#include <opentxs/core/crypto/OTCachedKey.hpp>
#include <opentxs/core/crypto/OTCrypto.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/core/crypto/OTSymmetricKey.hpp>
#include <bitcoin-base58/hash.h>
#include <cstring>
#include <iostream>

namespace opentxs
{

Identifier::Identifier()
    : OTData()
{
}

Identifier::Identifier(const Identifier& theID)
    : OTData(theID)
{
}

Identifier::Identifier(const char* szStr)
    : OTData()
{
    OT_ASSERT(nullptr != szStr);
    SetString(szStr);
}

Identifier::Identifier(const std::string& theStr)
    : OTData()
{
    OT_ASSERT(!theStr.empty());
    SetString(theStr.c_str());
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

void Identifier::SetString(const char* szString)
{
    OT_ASSERT(nullptr != szString);
    const String theStr(szString);
    SetString(theStr);
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

Identifier::~Identifier()
{
}

// When calling SignContract or VerifySignature with "HASH256" as the hash type,
// the signature will use (sha256 . sha256) as a message digest.
// In this case, SignContractDefaultHash and VerifyContractDefaultHash are used,
// which resort to low level calls to accomplish non standard message digests.
// Otherwise, it will use whatever OpenSSL provides by that name (see
// GetOpenSSLDigestByName).
const String Identifier::DefaultHashAlgorithm("HASH256");

// This method implements the (ripemd160 . sha256) hash,
// so the result is 20 bytes long.
bool Identifier::CalculateDigest(const unsigned char* data, size_t len)
{
    // The Hash160 function comes from the Bitcoin reference client, where
    // it is implemented as RIPEMD160 ( SHA256 ( x ) ) => 20 byte hash
    auto hash160 = Hash160(data, data + len);
    SetSize(20);
    memcpy(const_cast<void*>(GetPointer()), hash160, 20);
    return true;
}

bool Identifier::CalculateDigest(const String& strInput)
{
    return CalculateDigest(
        reinterpret_cast<const unsigned char*>(strInput.Get()),
        static_cast<size_t>(strInput.GetLength()));
}

bool Identifier::CalculateDigest(const OTData& dataInput)
{
    auto dataPtr = static_cast<const unsigned char*>(dataInput.GetPointer());
    return CalculateDigest(dataPtr, dataInput.GetSize());
}

// SET (binary id) FROM ENCODED STRING
//
void Identifier::SetString(const String& theStr)
{
    CryptoEngine::Util()->SetIDFromEncoded(theStr, *this);
}

// This Identifier is stored in binary form.
// But what if you want a pretty string version of it?
// Just call this function.
//
void Identifier::GetString(String& theStr) const
{
    CryptoEngine::Util()->EncodeID(*this, theStr); // *this input, theStr output.
}

} // namespace opentxs
