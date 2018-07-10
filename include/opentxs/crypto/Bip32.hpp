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

#ifndef OPENTXS_CRYPTO_BIP32_HPP
#define OPENTXS_CRYPTO_BIP32_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs::crypto
{
std::string Print(const proto::HDPath& node);

class Bip32
{
public:
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const std::uint32_t index) const = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> GetChild(
        const proto::AsymmetricKey& parent,
        const std::uint32_t index) const = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> GetHDKey(
        const EcdsaCurve& curve,
        const OTPassword& seed,
        proto::HDPath& path) const = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> GetPaymentCode(
        std::string& fingerprint,
        const std::uint32_t nym) const = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> GetStorageKey(
        std::string& seed) const = 0;
    EXPORT virtual std::string Seed(
        const std::string& fingerprint = "") const = 0;
    EXPORT virtual std::string SeedToFingerprint(
        const EcdsaCurve& curve,
        const OTPassword& seed) const = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> SeedToPrivateKey(
        const EcdsaCurve& curve,
        const OTPassword& seed) const = 0;
};
}  // namespace opentxs::crypto
#endif  // OPENTXS_CRYPTO_BIP32_HPP
