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

#ifndef OPENTXS_CORE_CRYPTO_TREZOR_CRYPTO_HPP
#define OPENTXS_CORE_CRYPTO_TREZOR_CRYPTO_HPP

#include "opentxs/core/Types.hpp"
#include "opentxs/core/crypto/Bip32.hpp"
#include "opentxs/core/crypto/Bip39.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"

extern "C" {
    #include <trezor-crypto/bip32.h>
}

#include <memory>
#include <string>

namespace opentxs
{

class OTPassword;

class TrezorCrypto : public Bip39, public Bip32
{
private:
    typedef bool DerivationMode;
    const DerivationMode DERIVE_PRIVATE = true;
    const DerivationMode DERIVE_PUBLIC = false;

    static std::string CurveName(const EcdsaCurve& curve);

    static std::unique_ptr<HDNode> InstantiateHDNode(
        const EcdsaCurve& curve,
        const OTPassword& seed);
    static std::unique_ptr<HDNode> InstantiateHDNode(const EcdsaCurve& curve);

    std::unique_ptr<HDNode> SerializedToHDNode(
        const proto::AsymmetricKey& serialized) const;
    serializedAsymmetricKey HDNodeToSerialized(
        const proto::AsymmetricKeyType& type,
        const HDNode& node,
        const DerivationMode privateVersion) const;
public:
    std::string toWords(const OTPassword& seed) const override;
    void WordsToSeed(
        const std::string words,
        OTPassword& seed,
        const std::string passphrase = Bip39::DEFAULT_PASSPHRASE) const override;
    std::string SeedToFingerprint(
        const EcdsaCurve& curve,
        const OTPassword& seed) const override;
    serializedAsymmetricKey SeedToPrivateKey(
        const EcdsaCurve& curve,
        const OTPassword& seed) const override;
    serializedAsymmetricKey GetChild(
        const proto::AsymmetricKey& parent,
        const uint32_t index) const override;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_TREZOR_CRYPTO_HPP
