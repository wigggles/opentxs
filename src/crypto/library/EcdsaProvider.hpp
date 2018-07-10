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

#ifndef IMPLEMENTATION_OPENTXS_CRYPTO_LIBRARY_ECDSAPROVIDER_HPP
#define IMPLEMENTATION_OPENTXS_CRYPTO_LIBRARY_ECDSAPROVIDER_HPP

#include "Internal.hpp"

#include "opentxs/crypto/library/EcdsaProvider.hpp"

namespace opentxs::crypto::implementation
{
class EcdsaProvider : virtual public crypto::EcdsaProvider
{
public:
    bool AsymmetricKeyToECPrivatekey(
        const AsymmetricKeyEC& asymmetricKey,
        const OTPasswordData& passwordData,
        OTPassword& privkey) const override;
    bool DecryptSessionKeyECDH(
        const AsymmetricKeyEC& privateKey,
        const AsymmetricKeyEC& publicKey,
        const OTPasswordData& password,
        SymmetricKey& sessionKey) const override;
    bool ECPrivatekeyToAsymmetricKey(
        const OTPassword& privkey,
        const OTPasswordData& passwordData,
        AsymmetricKeyEC& asymmetricKey) const override;
    bool ECPubkeyToAsymmetricKey(
        const Data& pubkey,
        AsymmetricKeyEC& asymmetricKey) const override;
    bool EncryptSessionKeyECDH(
        const AsymmetricKeyEC& privateKey,
        const AsymmetricKeyEC& publicKey,
        const OTPasswordData& passwordData,
        SymmetricKey& sessionKey,
        OTPassword& newKeyPassword) const override;
    bool ExportECPrivatekey(
        const OTPassword& privkey,
        const OTPasswordData& password,
        AsymmetricKeyEC& asymmetricKey) const override;
    bool ImportECPrivatekey(
        const proto::Ciphertext& asymmetricKey,
        const OTPasswordData& password,
        OTPassword& privkey) const override;
    bool PrivateToPublic(
        const proto::AsymmetricKey& privateKey,
        proto::AsymmetricKey& publicKey) const override;
    bool PrivateToPublic(const proto::Ciphertext& privateKey, Data& publicKey)
        const override;
    bool SeedToCurveKey(
        const OTPassword& seed,
        OTPassword& privateKey,
        Data& publicKey) const override;

    virtual ~EcdsaProvider() = default;

protected:
    bool AsymmetricKeyToECPubkey(
        const AsymmetricKeyEC& asymmetricKey,
        Data& pubkey) const;
    bool AsymmetricKeyToECPrivkey(
        const proto::Ciphertext& asymmetricKey,
        const OTPasswordData& passwordData,
        OTPassword& privkey) const;

    EcdsaProvider() = default;

private:
    virtual bool ECDH(
        const Data& publicKey,
        const OTPassword& privateKey,
        OTPassword& secret) const = 0;
    virtual bool ScalarBaseMultiply(const OTPassword& seed, Data& publicKey)
        const = 0;

    EcdsaProvider(const EcdsaProvider&) = delete;
    EcdsaProvider(EcdsaProvider&&) = delete;
    EcdsaProvider& operator=(const EcdsaProvider&) = delete;
    EcdsaProvider& operator=(EcdsaProvider&&) = delete;
};
}  // namespace opentxs::crypto::implementation
#endif  // IMPLEMENTATION_OPENTXS_CRYPTO_LIBRARY_ECDSAPROVIDER_HPP
