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

#ifndef OPENTXS_API_CRYPTO_ENCODE_HPP
#define OPENTXS_API_CRYPTO_ENCODE_HPP

#include "opentxs/Version.hpp"

#include <cstdint>
#include <string>

namespace opentxs
{

class Data;
class String;

namespace api
{
namespace crypto
{
class Encode
{
public:
    virtual std::string DataEncode(const std::string& input) const = 0;
    virtual std::string DataEncode(const Data& input) const = 0;
    virtual std::string DataDecode(const std::string& input) const = 0;
    virtual std::string IdentifierEncode(const Data& input) const = 0;
    virtual std::string IdentifierDecode(const std::string& input) const = 0;
    virtual bool IsBase62(const std::string& str) const = 0;
    virtual String Nonce(const std::uint32_t size) const = 0;
    virtual String Nonce(const std::uint32_t size, Data& rawOutput) const = 0;
    virtual std::string RandomFilename() const = 0;
    virtual std::string SanatizeBase58(const std::string& input) const = 0;
    virtual std::string SanatizeBase64(const std::string& input) const = 0;

    virtual ~Encode() = default;

protected:
    Encode() = default;

private:
    Encode(const Encode&) = delete;
    Encode(Encode&&) = delete;
    Encode& operator=(const Encode&) = delete;
    Encode& operator=(Encode&&) = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CRYPTO_ENCODE_HPP
