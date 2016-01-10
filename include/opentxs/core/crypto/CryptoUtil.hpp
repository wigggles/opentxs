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

#include <opentxs/core/String.hpp>

namespace opentxs
{

class Identifier;
class OTData;
class OTPassword;

class CryptoUtil
{

protected:
    CryptoUtil() = default;
    virtual bool GetPasswordFromConsole(
        OTPassword& theOutput, const char* szPrompt) const = 0;

public:
    virtual ~CryptoUtil() = default;
    EXPORT bool GetPasswordFromConsole(OTPassword& theOutput,
                                       bool bRepeat = false) const;
    virtual bool RandomizeMemory(uint8_t* szDestination,
                                 uint32_t nNewSize) const = 0;
    virtual void EncodeID(const Identifier& theInput,
                          String& strOutput) const = 0;
    virtual void SetIDFromEncoded(const String& strInput,
                                  Identifier& theOutput) const = 0;
    bool IsBase62(const std::string& str) const;
    // Caller is responsible to delete. TODO: return a unique pointer.
    // NOTE: the 'int32_t' here is very worrying to me. The reason it's
    // here is because that's what OpenSSL uses. So we may need to find
    // another way of doing it, so we can use a safer parameter here
    // than what it currently is. TODO security.
    virtual char* Base64Encode(const uint8_t* input, int32_t in_len,
                               bool bLineBreaks) const = 0;
    virtual uint8_t* Base64Decode(const char* input, size_t* out_len,
                                  bool bLineBreaks) const = 0;
    std::string RandomFilename() const;
    String Nonce(const uint32_t size) const;
    String Nonce(const uint32_t size, OTData& rawOutput) const;

    static String Base58CheckEncode(const OTPassword& input);
    static String Base58CheckEncode(const OTData& input);
    static bool Base58CheckDecode(const String& input, OTData& output);
    static bool Base58CheckDecode(const String& input, OTPassword& output);
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_CRYPTOUTIL_HPP
