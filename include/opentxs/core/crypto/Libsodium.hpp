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

#ifndef OPENTXS_CORE_CRYPTO_LIBSODIUM_HPP
#define OPENTXS_CORE_CRYPTO_LIBSODIUM_HPP

#include "opentxs/core/Proto.hpp"
#include "opentxs/core/crypto/Crypto.hpp"
#include "opentxs/core/crypto/CryptoAsymmetric.hpp"
#include "opentxs/core/crypto/CryptoHash.hpp"
#include "opentxs/core/crypto/Ecdsa.hpp"

namespace opentxs
{

class OTAsymmetricKey;
class OTData;
class OTPassword;
class OTPasswordData;

class Libsodium : public Crypto, public CryptoAsymmetric, public Ecdsa
{
    friend class CryptoEngine;

private:
    Libsodium() = default;

    void Cleanup_Override() const override {}
    bool ECDH(
        const OTData& publicKey,
        const OTPassword& seed,
        OTPassword& secret) const override;
    bool ExpandSeed(
        const OTPassword& seed,
        OTPassword& privateKey,
        OTData& publicKey) const;
    void Init_Override() const override;
    bool ScalarBaseMultiply(
        const OTPassword& seed,
        OTData& publicKey) const override;

public:
    bool RandomKeypair(
        OTPassword& privateKey,
        OTData& publicKey) const override;
    bool Sign(
        const OTData& plaintext,
        const OTAsymmetricKey& theKey,
        const proto::HashType hashType,
        OTData& signature, // output
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr) const override;
    bool Verify(
        const OTData& plaintext,
        const OTAsymmetricKey& theKey,
        const OTData& signature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const override;

    virtual ~Libsodium() = default;
};
}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_OTCRYPTO_HPP
