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

#ifndef OPENTXS_CRYPTO_BIP39_HPP
#define OPENTXS_CRYPTO_BIP39_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_WITH_BIP39
#include "opentxs/Proto.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{
namespace crypto
{
class Bip39
{
public:
    EXPORT virtual std::string DefaultSeed() const = 0;
    EXPORT virtual std::string ImportSeed(
        const OTPassword& words,
        const OTPassword& passphrase) const = 0;
    EXPORT virtual std::string NewSeed() const = 0;
    EXPORT virtual std::string Passphrase(
        const std::string& fingerprint = "") const = 0;
    EXPORT virtual std::shared_ptr<OTPassword> Seed(
        std::string& fingerprint,
        std::uint32_t& index) const = 0;
    EXPORT virtual bool UpdateIndex(
        std::string& seed,
        const std::uint32_t index) const = 0;
    EXPORT virtual std::string Words(
        const std::string& fingerprint = "") const = 0;

    EXPORT virtual ~Bip39() = default;

protected:
    Bip39() = default;

private:
    Bip39(const Bip39&) = delete;
    Bip39(Bip39&&) = delete;
    Bip39& operator=(const Bip39&) = delete;
    Bip39& operator=(Bip39&&) = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif  // OT_CRYPTO_WITH_BIP39
#endif  // OPENTXS_CRYPTO_BIP39_HPP
