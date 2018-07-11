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

#ifndef OPENTXS_CORE_CRYPTO_ASYMMETRICKEYEC_HPP
#define OPENTXS_CORE_CRYPTO_ASYMMETRICKEYEC_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/crypto/key/Asymmetric.hpp"

#include <memory>

namespace opentxs
{
namespace crypto
{
namespace key
{
class EllipticCurve : public Asymmetric
{
private:
    friend class crypto::EcdsaProvider;

    typedef Asymmetric ot_super;

protected:
    OTData key_;
    std::unique_ptr<proto::Ciphertext> encrypted_key_{nullptr};
    std::shared_ptr<proto::HDPath> path_{nullptr};
    std::unique_ptr<proto::Ciphertext> chain_code_{nullptr};

    EllipticCurve() = delete;
    explicit EllipticCurve(
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role);
    explicit EllipticCurve(const proto::AsymmetricKey& serializedKey);
    explicit EllipticCurve(
        const proto::AsymmetricKeyType keyType,
        const String& publicKey);

    void ReleaseKeyLowLevel_Hook() const override {}

public:
    bool IsEmpty() const override;
    virtual const crypto::EcdsaProvider& ECDSA() const = 0;
    bool GetKey(Data& key) const;
    bool GetKey(proto::Ciphertext& key) const;
    bool GetPublicKey(String& strKey) const override;
    bool GetPublicKey(Data& key) const;
    using ot_super::Path;
    const std::string Path() const override;
    bool Path(proto::HDPath& output) const override;
    bool ReEncryptPrivateKey(
        const OTPassword& theExportPassword,
        bool bImporting) const override;
    void Release_EllipticCurve() {}
    void Release() override;
    bool SetKey(const Data& key);
    bool SetKey(std::unique_ptr<proto::Ciphertext>& key);
    std::shared_ptr<proto::AsymmetricKey> Serialize() const override;
    bool TransportKey(Data& publicKey, OTPassword& privateKey) const override;

    virtual ~EllipticCurve();
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_ASYMMETRICKEYEC_HPP
