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

#ifndef OPENTXS_API_CRYPTO_HASH_HPP
#define OPENTXS_API_CRYPTO_HASH_HPP

#include "opentxs/Version.hpp"

#include "opentxs/Proto.hpp"

#include <cstdint>
#include <string>

namespace opentxs
{
class Data;
class OTPassword;
class String;

namespace api
{
namespace crypto
{

class Hash
{
public:
    virtual bool Digest(
        const proto::HashType hashType,
        const OTPassword& data,
        OTPassword& digest) const = 0;
    virtual bool Digest(
        const proto::HashType hashType,
        const Data& data,
        Data& digest) const = 0;
    virtual bool Digest(
        const proto::HashType hashType,
        const String& data,
        Data& digest) const = 0;
    virtual bool Digest(
        const std::uint32_t type,
        const std::string& data,
        std::string& encodedDigest) const = 0;
    virtual bool HMAC(
        const proto::HashType hashType,
        const OTPassword& key,
        const Data& data,
        OTPassword& digest) const = 0;

    virtual ~Hash() = default;

protected:
    Hash() = default;

private:
    Hash(const Hash&) = delete;
    Hash(Hash&&) = delete;
    Hash& operator=(const Hash&) = delete;
    Hash& operator=(Hash&&) = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CRYPTO_HASH_HPP
