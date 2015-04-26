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

#ifndef OPENTXS_CORE_CRYPTO_BITCOINCRYPTO_HPP
#define OPENTXS_CORE_CRYPTO_BITCOINCRYPTO_HPP

#include <bitcoin-base58/base58.h>
#include <vector>

namespace opentxs
{

// This class is built in the same fashion as the Bitcoin reference client's
// bitcoin address class:
// https://github.com/bitcoin/bitcoin/blob/0.9.3/src/base58.h#L101
class IdentifierFormat : public CBase58Data
{
private:
    // Prefix to all IDs.
    const std::string prefix_;
    // version byte prepended before base58Check encoding.
    // Similar to https://en.bitcoin.it/wiki/Base58Check_encoding#Version_bytes.
    const unsigned char versionByte_;

    // do not expose this version of CBase58Data's SetString.
    bool SetString(const char* psz, unsigned int nVersionBytes = 1);

public:
    IdentifierFormat();

    // Checks for OT prefix before calling base method.
    // Decodes the OT ID format. Access the binary data with Get().
    bool SetString(const std::string& str);
    // Prepends OT prefix.
    std::string ToString() const;

    // Encodes to the OT ID format. Access the encoded string
    // with ToString().
    bool Set(const unsigned char* pbegin, const unsigned char* pend);
    // Get back data that is in OT format.
    // Use SetString() beforehand.
    std::vector<unsigned char> Get() const;

    // Convenience functions for using this class.
    static std::string encode(const unsigned char* data, size_t len);
    static bool decode(const std::string& str,
                       std::vector<unsigned char>& decoded);
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_BITCOINCRYPTO_HPP
