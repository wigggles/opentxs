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

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOENCODING_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOENCODING_HPP

#include "opentxs/core/String.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace opentxs
{

class CryptoEngine;
class OTData;
class OTPassword;

class CryptoEncodingEngine
{
private:
    friend class CryptoEngine;

    std::string IdentifierEncode(const OTPassword& input) const;
    std::string SanatizeBase58(const std::string& input) const;

protected:
    typedef std::vector<unsigned char> Data;

    static const std::uint8_t LineWidth{72};

    std::string Base58CheckEncode(
        const std::uint8_t* inputStart,
        const size_t& inputSize,
        const bool& breakLines = false) const;
    bool Base58CheckDecode(
        const std::string&& input,
          Data & output) const;
    std::string BreakLines(const std::string& input) const;

    CryptoEncodingEngine() = default;

public:
    std::string DataEncode(
        const std::string& input,
        const bool& breakLines = false) const;
    std::string DataEncode(
        const OTData& input,
        const bool& breakLines = false) const;
    std::string DataDecode(const std::string& input) const;
    std::string IdentifierEncode(const OTData& input) const;
    std::string IdentifierDecode(const std::string& input) const;
    bool IsBase62(const std::string& str) const;
    String Nonce(const uint32_t size) const;
    String Nonce(const uint32_t size, OTData& rawOutput) const;
    std::string RandomFilename() const;

    ~CryptoEncodingEngine() = default;
};
} // namespace opentxs
#endif // OPENTXS_CORE_CRYPTO_CRYPTOENCODING_HPP
