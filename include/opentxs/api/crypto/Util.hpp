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

#ifndef OPENTXS_API_CRYPTO_UTIL_HPP
#define OPENTXS_API_CRYPTO_UTIL_HPP

#include "opentxs/Forward.hpp"

#include <cstdint>

namespace opentxs
{
namespace api
{
namespace crypto
{

class Util
{
public:
    virtual bool RandomizeMemory(
        std::uint8_t* szDestination,
        std::uint32_t nNewSize) const = 0;
    virtual bool GetPasswordFromConsole(
        OTPassword& theOutput,
        bool bRepeat = false) const = 0;

    virtual ~Util() = default;

protected:
    Util() = default;

private:
    Util(const Util&) = delete;
    Util(Util&&) = delete;
    Util& operator=(const Util&) = delete;
    Util& operator=(Util&&) = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CRYPTO_UTIL_HPP
