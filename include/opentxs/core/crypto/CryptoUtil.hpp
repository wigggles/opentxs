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

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOUTIL_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOUTIL_HPP

#include "opentxs/core/String.hpp"

#include <cstdint>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

namespace opentxs
{

class Identifier;
class OTData;
class OTPassword;
class String;

class CryptoUtil
{
private:
    typedef std::vector<unsigned char> DecodedOutput;

    static const std::uint8_t LineWidth{72};

    static std::string Base58CheckEncode(
        const std::uint8_t* inputStart,
        const size_t& inputSize,
        const bool& breakLines = false);
    static bool Base58CheckDecode(
        const std::string&& input,
        DecodedOutput& output);
    static std::string BreakLines(const std::string& input);
    static std::string Sanatize(const std::string& input);

protected:
    CryptoUtil() = default;
    virtual bool GetPasswordFromConsole(
        OTPassword& theOutput, const char* szPrompt) const = 0;

public:
    static std::string Base58CheckEncode(
        const std::string& input,
        const bool& breakLines = false);
    static std::string Base58CheckEncode(
        const OTData& input,
        const bool& breakLines = false);
    static std::string Base58CheckEncode(const OTPassword& input);

    static std::string Base58CheckDecode(const std::string&& input);
    static bool Base58CheckDecode(const String& input, OTData& output);
    static bool Base58CheckDecode(const String& input, OTPassword& output);

    virtual bool RandomizeMemory(uint8_t* szDestination,
                                 uint32_t nNewSize) const = 0;
    bool GetPasswordFromConsole(OTPassword& theOutput,
                                       bool bRepeat = false) const;

    bool IsBase62(const std::string& str) const;

    std::string RandomFilename() const;
    String Nonce(const uint32_t size) const;
    String Nonce(const uint32_t size, OTData& rawOutput) const;

    virtual ~CryptoUtil() = default;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_CRYPTOUTIL_HPP
