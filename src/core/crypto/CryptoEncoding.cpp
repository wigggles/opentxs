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

#include "opentxs/core/crypto/CryptoEncoding.hpp"

#include "opentxs/core/crypto/BitcoinCrypto.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/OTData.hpp"

#include <iostream>
#include <regex>

namespace opentxs
{

std::string CryptoEncoding::Base58CheckEncode(
    const std::uint8_t* inputStart,
    const size_t& size,
    const bool& breakLines) const
{
    if (breakLines) {
        return BreakLines(::EncodeBase58Check(inputStart, inputStart + size));
    } else {
        return ::EncodeBase58Check(inputStart, inputStart + size);
    }
}

bool CryptoEncoding::Base58CheckDecode(
    const std::string&& input,
    DecodedOutput& output) const
{
    return ::DecodeBase58Check(input.c_str(), output);
}

std::string CryptoEncoding::BreakLines(const std::string& input) const
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

    if (0 != width) {
        output.push_back('\n');
    }

    return output;
}

std::string CryptoEncoding::DataEncode(
    const std::string& input,
    const bool& breakLines) const
{
    return Base58CheckEncode(
        reinterpret_cast<const uint8_t*>(input.c_str()),
        input.size(),
        breakLines);
}

std::string CryptoEncoding::DataEncode(
    const OTData& input,
    const bool& breakLines) const
{
    return Base58CheckEncode(
        static_cast<const uint8_t*>(input.GetPointer()),
        input.GetSize(),
        breakLines);
}

std::string CryptoEncoding::DataDecode(const std::string& input) const
{
    DecodedOutput decoded;

    if (Base58CheckDecode(SanatizeBase58(input), decoded)) {

        return std::string(
            reinterpret_cast<const char*>(decoded.data()), decoded.size());
    }

    return "";
}

std::string CryptoEncoding::IdentifierEncode(
    const OTData& input) const
{
    return Base58CheckEncode(
        static_cast<const uint8_t*>(input.GetPointer()),
        input.GetSize(),
        false);
}

std::string CryptoEncoding::IdentifierEncode(const OTPassword& input) const
{
    if (input.isMemory()) {
        return Base58CheckEncode(
            static_cast<const uint8_t*>(input.getMemory()),
            input.getMemorySize(),
            false);
    } else {
        return Base58CheckEncode(
            reinterpret_cast<const uint8_t*>(input.getPassword()),
            input.getPasswordSize(),
            false);
    }
}

std::string CryptoEncoding::IdentifierDecode(const std::string& input) const
{
    DecodedOutput decoded;

    if (Base58CheckDecode(SanatizeBase58(input), decoded)) {

        return std::string(
            reinterpret_cast<const char*>(decoded.data()), decoded.size());
    }

    return "";
}

bool CryptoEncoding::IsBase62(const std::string& str) const
{
    return str.find_first_not_of(
               "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHI"
               "JKLMNOPQRSTUVWXYZ") == std::string::npos;
}

String CryptoEncoding::Nonce(const uint32_t size) const
{
    OTData unusedOutput;
    return Nonce(size, unusedOutput);
}

String CryptoEncoding::Nonce(const uint32_t size, OTData& rawOutput) const
{
    rawOutput.zeroMemory();
    rawOutput.SetSize(size);

    OTPassword source;
    source.randomizeMemory(size);

    String nonce(IdentifierEncode(source));

    rawOutput.Assign(source.getMemory(), source.getMemorySize());
    return nonce;
}

std::string CryptoEncoding::RandomFilename() const { return Nonce(16).Get(); }

std::string CryptoEncoding::SanatizeBase58(const std::string& input) const
{
    return std::regex_replace(input, std::regex("[^1-9A-HJ-NP-Za-km-z]"), "");
}
}  // namespace opentxs
