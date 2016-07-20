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

#include "opentxs/core/crypto/Ecdsa.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"

#include <memory>

namespace opentxs
{

class OTData;
class OTPassword;
class String;

class AsymmetricKeyEC : public OTAsymmetricKey
{
private:
    typedef OTAsymmetricKey ot_super;

protected:
    std::unique_ptr<OTData> key_;

    AsymmetricKeyEC() = delete;
    explicit AsymmetricKeyEC(
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role);
    explicit AsymmetricKeyEC(const proto::AsymmetricKey& serializedKey);
    explicit AsymmetricKeyEC(
        const proto::AsymmetricKeyType keyType,
        const String& publicKey);

    virtual Ecdsa& ECDSA() const = 0;
    void ReleaseKeyLowLevel_Hook() const override {}

public:
    bool IsEmpty() const override;
    bool GetKey(OTData& key) const override;
    bool GetPublicKey(String& strKey) const override;
    bool ReEncryptPrivateKey(
        const OTPassword& theExportPassword,
        bool bImporting) const override;
    void Release_AsymmetricKeyEC() {}
    void Release() override;
    bool SetKey(std::unique_ptr<OTData>& key, bool isPrivate) override;
    serializedAsymmetricKey Serialize() const override;
    bool TransportKey(unsigned char* publicKey, unsigned char* privateKey)
        const override;

    virtual ~AsymmetricKeyEC() = default;
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_ASYMMETRICKEYEC_HPP
