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

#include "opentxs/Version.hpp"

#include "opentxs/core/Types.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace opentxs
{

class CryptoEncoding
{
protected:
    CryptoEncoding() = default;

public:
    virtual std::string Base58CheckEncode(
        const std::uint8_t* inputStart,
        const std::size_t& inputSize) const = 0;
    virtual bool Base58CheckDecode(const std::string&& input, RawData& output)
        const = 0;

    virtual ~CryptoEncoding() = default;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_CRYPTOENCODING_HPP
