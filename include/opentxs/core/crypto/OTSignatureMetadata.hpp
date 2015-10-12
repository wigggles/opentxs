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

#ifndef OPENTXS_CORE_CRYPTO_OTSIGNATUREMETADATA_HPP
#define OPENTXS_CORE_CRYPTO_OTSIGNATUREMETADATA_HPP

namespace opentxs
{

class OTSignatureMetadata
{
public:
    OTSignatureMetadata();

    bool operator==(const OTSignatureMetadata& rhs) const;

    bool operator!=(const OTSignatureMetadata& rhs) const
    {
        return !(operator==(rhs));
    }

    bool SetMetadata(char metaKeyType, char metaNymID, char metaMasterCredID,
                     char metaChildCredID);

    inline bool HasMetadata() const
    {
        return hasMetadata_;
    }

    inline char GetKeyType() const
    {
        return metaKeyType_;
    }

    inline char FirstCharNymID() const
    {
        return metaNymID_;
    }

    inline char FirstCharMasterCredID() const
    {
        return metaMasterCredID_;
    }

    inline char FirstCharChildCredID() const
    {
        return metaChildCredID_;
    }

private:
    // Defaults to false. Is set true by calling SetMetadata
    bool hasMetadata_;
    // Can be A, E, or S (authentication, encryption, or signing.
    // Also, E would be unusual.)
    char metaKeyType_;
    // Can be any letter from base62 alphabet. Represents
    // first letter of a Nym's ID.
    char metaNymID_;
    // Can be any letter from base62 alphabet.
    // Represents first letter of a Master Credential
    // ID (for that Nym.)
    char metaMasterCredID_;
    // Can be any letter from base62 alphabet. Represents
    // first letter of a Credential ID (signed by that Master.)
    char metaChildCredID_;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTSIGNATUREMETADATA_HPP
