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

#ifndef OPENTXS_CRYPTO_LIBRARY_RIPEMD160_HPP
#define OPENTXS_CRYPTO_LIBRARY_RIPEMD160_HPP

#include "Internal.hpp"

#include <cstdint>

namespace opentxs::crypto
{
class Ripemd160
{
public:
    OPENTXS_EXPORT virtual bool RIPEMD160(
        const std::uint8_t* input,
        const std::size_t inputSize,
        std::uint8_t* output) const = 0;

    OPENTXS_EXPORT virtual ~Ripemd160() = default;

protected:
    Ripemd160() = default;

private:
    Ripemd160(const Ripemd160&) = delete;
    Ripemd160(Ripemd160&&) = delete;
    Ripemd160& operator=(const Ripemd160&) = delete;
    Ripemd160& operator=(Ripemd160&&) = delete;
};
}  // namespace opentxs::crypto
#endif
