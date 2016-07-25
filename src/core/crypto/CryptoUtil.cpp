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

#include "opentxs/core/crypto/CryptoUtil.hpp"

#include "opentxs/core/crypto/BitcoinCrypto.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/core/String.hpp"

#include <stdint.h>
#include <cstdint>
#include <iostream>
#include <regex>
#include <string>


namespace opentxs
{

std::string CryptoUtil::BreakLines(const std::string& input)
{
    std::string output;
    size_t width = 0;

    for (auto& character : input) {
        output.push_back(character);

        if (++width >= LineWidth) {
            output.push_back('\n');
            width = 0;
        }
    }
    output.push_back('\n');

    return output;
}

std::string CryptoUtil::Sanatize(const std::string& input)
{
    return std::regex_replace(input, std::regex("[^1-9A-HJ-NP-Za-km-z]"), "");
}

bool CryptoUtil::GetPasswordFromConsole(OTPassword& theOutput, bool bRepeat)
    const
{
    int32_t nAttempts = 0;

    for (;;) {
        theOutput.zeroMemory();

        if (GetPasswordFromConsole(theOutput, "(OT) passphrase: ")) {
            if (!bRepeat) {
                std::cout << std::endl;
                return true;
            }
        } else {
            std::cout << "Sorry." << std::endl;
            return false;
        }

        OTPassword tempPassword;

        if (!GetPasswordFromConsole(
                tempPassword, "(Verifying) passphrase again: ")) {
            std::cout << "Sorry." << std::endl;
            return false;
        }

        if (!tempPassword.Compare(theOutput)) {
            if (++nAttempts >= 3) break;

            std::cout << "(Mismatch, try again.)\n" << std::endl;
        } else {
            std::cout << std::endl;
            return true;
        }
    }

    std::cout << "Sorry." << std::endl;

    return false;
}

bool CryptoUtil::IsBase62(const std::string& str) const
{
    return str.find_first_not_of(
               "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHI"
               "JKLMNOPQRSTUVWXYZ") == std::string::npos;
}

std::string CryptoUtil::RandomFilename() const { return Nonce(16).Get(); }

String CryptoUtil::Nonce(const uint32_t size) const
{
    OTData unusedOutput;
    return Nonce(size, unusedOutput);
}

String CryptoUtil::Nonce(const uint32_t size, OTData& rawOutput) const
{
    rawOutput.zeroMemory();
    rawOutput.SetSize(size);

    OTPassword source;
    source.randomizeMemory(size);

    String nonce(Base58CheckEncode(source));

    rawOutput.Assign(source.getMemory(), source.getMemorySize());
    return nonce;
}

std::string CryptoUtil::Base58CheckEncode(
    const std::uint8_t* inputStart,
    const size_t& size,
    const bool& breakLines)
{
    if (breakLines) {
        return BreakLines(::EncodeBase58Check(inputStart, inputStart + size));
    } else {
        return ::EncodeBase58Check(inputStart, inputStart + size);
    }
}

std::string CryptoUtil::Base58CheckEncode(
    const std::string& input,
    const bool& breakLines)
{
    return Base58CheckEncode(
        reinterpret_cast<const uint8_t*>(input.c_str()),
        input.size(),
        breakLines);
}

std::string CryptoUtil::Base58CheckEncode(
    const OTData& input,
    const bool& breakLines)
{
    return Base58CheckEncode(
        static_cast<const uint8_t*>(input.GetPointer()),
        input.GetSize(),
        breakLines);
}

std::string CryptoUtil::Base58CheckEncode(const OTPassword& input)
{
    return Base58CheckEncode(
        static_cast<const uint8_t*>(input.getMemory()),
        input.getMemorySize(),
        false);
}

bool CryptoUtil::Base58CheckDecode(
    const std::string&& input,
    DecodedOutput& output)
{
    return ::DecodeBase58Check(input.c_str(), output);
}

std::string CryptoUtil::Base58CheckDecode(const std::string&& input)
{
    DecodedOutput decoded;

    if (Base58CheckDecode(Sanatize(input), decoded)) {

        return std::string(
            reinterpret_cast<const char*>(decoded.data()), decoded.size());
    }

    return "";
}

bool CryptoUtil::Base58CheckDecode(const String& input, OTData& output)
{
    DecodedOutput decoded;

    if (Base58CheckDecode(Sanatize(input.Get()), decoded)) {
        output.Assign(decoded.data(), decoded.size());

        return true;
    }

    return false;
}

bool CryptoUtil::Base58CheckDecode(const String& input, OTPassword& output)
{
    OTData decodedOutput;
    bool decoded = Base58CheckDecode(input, decodedOutput);

    if (decoded) {
        output.setMemory(decodedOutput);

        return true;
    }

    return false;
}
}  // namespace opentxs
