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

#include "opentxs/core/OTData.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"

#include <bitcoin-base58/base58.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>

namespace opentxs
{

bool CryptoUtil::GetPasswordFromConsole(OTPassword& theOutput, bool bRepeat) const
{
    int32_t nAttempts = 0;

    for (;;) {
        theOutput.zeroMemory();

        if (GetPasswordFromConsole(theOutput, "(OT) passphrase: ")) {
            if (!bRepeat) {
                std::cout << std::endl;
                return true;
            }
        }
        else {
            std::cout << "Sorry." << std::endl;
            return false;
        }

        OTPassword tempPassword;

        if (!GetPasswordFromConsole(tempPassword,
                                            "(Verifying) passphrase again: ")) {
            std::cout << "Sorry." << std::endl;
            return false;
        }

        if (!tempPassword.Compare(theOutput)) {
            if (++nAttempts >= 3) break;

            std::cout << "(Mismatch, try again.)\n" << std::endl;
        }
        else {
            std::cout << std::endl;
            return true;
        }
    }

    std::cout << "Sorry." << std::endl;

    return false;
}

bool CryptoUtil::IsBase62(const std::string& str) const
{
    return str.find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHI"
                                 "JKLMNOPQRSTUVWXYZ") == std::string::npos;
}

std::string CryptoUtil::RandomFilename() const
{
    return Nonce(16).Get();
}

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

String CryptoUtil::Base58CheckEncode(const OTData& input)
{
    OTPassword transformedInput;
    transformedInput.setMemory(input);

    return Base58CheckEncode(transformedInput);
}

String CryptoUtil::Base58CheckEncode(const OTPassword& input)
{
    const uint8_t* inputStart = static_cast<const uint8_t*>(input.getMemory());
    const uint8_t* inputEnd = inputStart + input.getMemorySize();

    String encodedInput = ::EncodeBase58Check(inputStart, inputEnd);
    return encodedInput;
}

bool CryptoUtil::Base58CheckDecode(const String& input, OTPassword& output)
{
    OTData decodedOutput;
    bool decoded = Base58CheckDecode(input, decodedOutput);

    if (decoded) {
        output.setMemory(decodedOutput);

        return true;
    } else {

        return false;
    }
}

bool CryptoUtil::Base58CheckDecode(const String& input, OTData& output)
{
    std::vector<unsigned char> decodedInput;
    bool decoded = DecodeBase58Check(input.Get(), decodedInput);

    if (decoded) {
        OTData dataOutput(decodedInput);
        output = dataOutput;

        return true;
    } else {

        return false;
    }
}


} // namespace opentxs
