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

#ifndef IMPLEMENTATION_OPENTXS_CRYPTO_KEY_KEYPAIR_HPP
#define IMPLEMENTATION_OPENTXS_CRYPTO_KEY_KEYPAIR_HPP

#include "Internal.hpp"

namespace opentxs::crypto::key::implementation
{
class Keypair : virtual public key::Keypair
{
public:
    bool CalculateID(Identifier& theOutput) const override;
    const Asymmetric& GetPrivateKey() const override;
    const Asymmetric& GetPublicKey() const override;
    bool GetPublicKey(String& strKey) const override;
    std::int32_t GetPublicKeyBySignature(
        Keys& listOutput,
        const OTSignature& theSignature,
        bool bInclusive = false) const override;
    bool hasCapability(const NymCapability& capability) const override;
    bool HasPrivateKey() const override;
    bool HasPublicKey() const override;
    bool ReEncrypt(const OTPassword& theExportPassword, bool bImporting)
        override;
    std::shared_ptr<proto::AsymmetricKey> Serialize(
        bool privateKey = false) const override;
    bool TransportKey(Data& publicKey, OTPassword& privateKey) const override;
    bool Verify(const Data& plaintext, const proto::Signature& sig)
        const override;

    ~Keypair() = default;

private:
    friend opentxs::LowLevelKeyGenerator;
    friend key::Keypair;

    OTAsymmetricKey m_pkeyPublic;
    OTAsymmetricKey m_pkeyPrivate;
    const proto::KeyRole role_{proto::KEYROLE_ERROR};

    Keypair* clone() const override;

    bool make_new_keypair(const NymParameters& nymParameters);

    Keypair(
        const NymParameters& nymParameters,
        const proto::KeyRole role = proto::KEYROLE_ERROR);
    Keypair(
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey);
    explicit Keypair(const proto::AsymmetricKey& serializedPubkey);
    Keypair() = delete;
    Keypair(const Keypair&);
    Keypair(Keypair&&) = delete;
    Keypair& operator=(const Keypair&) = delete;
    Keypair& operator=(Keypair&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // IMPLEMENTATION_OPENTXS_CRYPTO_KEY_KEYPAIR_HPP
