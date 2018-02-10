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

#ifndef OPENTXS_API_CRYPTO_IMPLEMENTATION_ENCODE_HPP
#define OPENTXS_API_CRYPTO_IMPLEMENTATION_ENCODE_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
namespace implementation
{
class Encode : virtual public api::crypto::Encode
{
public:
    std::string DataEncode(const std::string& input) const override;
    std::string DataEncode(const Data& input) const override;
    std::string DataDecode(const std::string& input) const override;
    std::string IdentifierEncode(const Data& input) const override;
    std::string IdentifierDecode(const std::string& input) const override;
    bool IsBase62(const std::string& str) const override;
    String Nonce(const std::uint32_t size) const override;
    String Nonce(const std::uint32_t size, Data& rawOutput) const override;
    std::string RandomFilename() const override;
    std::string SanatizeBase58(const std::string& input) const override;
    std::string SanatizeBase64(const std::string& input) const override;

    ~Encode() = default;

private:
    friend class api::implementation::Crypto;

    static const std::uint8_t LineWidth{72};

    CryptoEncoding& base58_;

    std::string Base64Encode(
        const std::uint8_t* inputStart,
        const std::size_t& inputSize) const;
    bool Base64Decode(const std::string&& input, RawData& output) const;
    std::string BreakLines(const std::string& input) const;
    std::string IdentifierEncode(const OTPassword& input) const;

    Encode() = delete;
    Encode(CryptoEncoding& base58);
    Encode(const Encode&) = delete;
    Encode& operator=(const Encode&) = delete;
};
}  // namespace implementation
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CRYPTO_IMPLEMENTATION_ENCODE_HPP
