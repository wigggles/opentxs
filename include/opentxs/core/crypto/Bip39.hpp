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

#ifndef OPENTXS_CORE_CRYPTO_BIP39_HPP
#define OPENTXS_CORE_CRYPTO_BIP39_HPP

#include <memory>
#include <string>

#include <opentxs-proto/verify/VerifyCredentials.hpp>

#include "opentxs/core/crypto/CryptoSymmetric.hpp"

namespace opentxs
{
class OTPassword;

class Bip39
{
private:
    static const CryptoSymmetric::Mode DEFAULT_ENCRYPTION_MODE;

    bool DecryptSeed(proto::Seed& seed) const;
    std::string SaveSeed(
        const std::string& words,
        const std::string& passphrase) const;
    bool SeedToData(const proto::Seed& seed, OTPassword& output) const;
    std::shared_ptr<proto::Seed> SerializedSeed(
        const std::string& fingerprint = "") const;

public:
    static const std::string DEFAULT_PASSPHRASE;

    std::string ImportSeed(
        const std::string& words,
        const std::string& passphrase) const;
    std::string NewSeed() const;
    std::string Passphrase(const std::string& fingerprint = "") const;
    std::shared_ptr<OTPassword> Seed(const std::string& fingerprint = "") const;
    std::string Words(const std::string& fingerprint = "") const;

    virtual std::string toWords(
        const OTPassword& seed) const = 0;
    virtual void WordsToSeed(
        const std::string words,
        OTPassword& seed,
        const std::string passphrase = DEFAULT_PASSPHRASE) const = 0;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_BIP39_HPP
