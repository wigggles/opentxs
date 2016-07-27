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

#ifndef OPENTXS_CORE_CRYPTO_ECDSA_HPP
#define OPENTXS_CORE_CRYPTO_ECDSA_HPP

#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/Proto.hpp"

namespace opentxs
{
class OTAsymmetricKey;
class OTData;
class OTPassword;
class OTPasswordData;

class Ecdsa
{
protected:
    Ecdsa() = default;

    virtual bool AsymmetricKeyToECPubkey(
        const OTAsymmetricKey& asymmetricKey,
        OTData& pubkey) const;
    virtual bool AsymmetricKeyToECPrivkey(
        const OTData& asymmetricKey,
        const OTPasswordData& passwordData,
        OTPassword& privkey,
        const OTPassword* exportPassword = nullptr) const;
    virtual bool ECDH(
        const OTData& publicKey,
        const OTPassword& privateKey,
        OTPassword& secret) const = 0;
    virtual bool ScalarBaseMultiply(
        const OTPassword& privateKey,
        OTData& publicKey) const = 0;

public:
    static const proto::HashType ECDHDefaultHMAC = proto::HASHTYPE_SHA256;

    static bool DecryptPrivateKey(
        const OTData& encryptedKey,
        const OTPassword& password,
        OTPassword& plaintextKey);
    static bool EncryptPrivateKey(
        const OTPassword& plaintextKey,
        const OTPassword& password,
        OTData& encryptedKey);

    virtual bool AsymmetricKeyToECPrivatekey(
        const OTAsymmetricKey& asymmetricKey,
        const OTPasswordData& passwordData,
        OTPassword& privkey,
        const OTPassword* exportPassword = nullptr) const;
    virtual bool DecryptSessionKeyECDH(
        const symmetricEnvelope& encryptedSessionKey,
        const OTPassword& privateKey,
        const OTData& publicKey,
        OTPassword& sessionKey) const;
    virtual bool ECPrivatekeyToAsymmetricKey(
        const OTPassword& privkey,
        const OTPasswordData& passwordData,
        OTAsymmetricKey& asymmetricKey) const;
    virtual bool ECPubkeyToAsymmetricKey(
        std::unique_ptr<OTData>& pubkey,
        OTAsymmetricKey& asymmetricKey) const;
    virtual bool EncryptSessionKeyECDH(
        const OTPassword& sessionKey,
        const OTPassword& privateKey,
        const OTData& publicKey,
        symmetricEnvelope& encryptedSessionKey) const;
    virtual bool ExportECPrivatekey(
        const OTPassword& privkey,
        const OTPassword& password,
        OTAsymmetricKey& asymmetricKey) const;
    virtual bool ImportECPrivatekey(
        const OTData& asymmetricKey,
        const OTPassword& password,
        OTPassword& privkey) const;
    virtual bool PrivateToPublic(
        const proto::AsymmetricKey& privateKey,
        proto::AsymmetricKey& publicKey) const;
    virtual bool RandomKeypair(
        OTPassword& privateKey,
        OTData& publicKey) const = 0;
    virtual bool SeedToCurveKey(
        const OTPassword& seed,
        OTPassword& privateKey,
        OTData& publicKey) const;

    virtual ~Ecdsa() = default;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_ECDSA_HPP
