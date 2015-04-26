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

#include <opentxs/core/crypto/BitcoinCrypto.hpp>

#include <vector>
#include <iostream>

namespace opentxs
{

// Rationale for prefix "ot" (and not "OT"):
// part of the base58 alphabet, like the string that follows.
// The chosen version byte, 137, is unused by Bitcoin, so the part
// following the prefix is not a valid BTC address.
IdentifierFormat::IdentifierFormat()
    : prefix_("ot")
    , versionByte_(137)
{
}

bool IdentifierFormat::SetString(const std::string& str)
{
    // Check for the OT prefix.
    if (str.compare(0, prefix_.size(), prefix_)) {
        // Prefix does not match.
        return false;
    }
    // The base function does base58 check decoding and checksum checking.
    return CBase58Data::SetString(str.substr(prefix_.size()));
}

std::string IdentifierFormat::ToString() const
{
    return prefix_ + CBase58Data::ToString();
}

bool IdentifierFormat::Set(const unsigned char* pbegin,
                           const unsigned char* pend)
{
    // We are using a single byte for the version.
    std::vector<unsigned char> versionPrefix(1);
    versionPrefix[0] = versionByte_;
    SetData(versionPrefix, pbegin, pend);
    return true;
}

std::vector<unsigned char> IdentifierFormat::Get() const
{
    return std::vector<unsigned char>(vchData.begin(), vchData.end());
}

std::string IdentifierFormat::encode(const unsigned char* data, size_t len)
{
    IdentifierFormat fmt;
    fmt.Set(data, data + len);
    return fmt.ToString();
}

bool IdentifierFormat::decode(const std::string& str,
                              std::vector<unsigned char>& decoded)
{
    IdentifierFormat fmt;
    bool success = fmt.SetString(str);
    if (!success) return false;
    decoded = fmt.Get();
    return true;
}

} // namespace opentxs
