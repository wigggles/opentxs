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

#ifndef OPENTXS_CORE_CRYPTO_ASYMMETRICKEYSECP256K1_HPP
#define OPENTXS_CORE_CRYPTO_ASYMMETRICKEYSECP256K1_HPP

#include <opentxs/core/FormattedKey.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/crypto/LowLevelKeyGenerator.hpp>

#include <bitcoin-base58/base58.h>

namespace opentxs
{

class OTCaller;
class OTPassword;
class String;

class AsymmetricKeySecp256k1 : public OTAsymmetricKey
{
private:
    typedef OTAsymmetricKey ot_super;
    friend class OTAsymmetricKey; // For the factory.
    friend class LowLevelKeyGenerator;

    AsymmetricKeySecp256k1();
    virtual void ReleaseKeyLowLevel_Hook() const;
    // used by LowLevelKeyGenerator
    bool SetKey(
        const OTPassword& key);
    FormattedKey key_;

public:
    virtual CryptoAsymmetric& engine() const;
    virtual bool IsEmpty() const;
    virtual bool SetPrivateKey(
        const FormattedKey& strCert,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);
    virtual bool SetPublicKeyFromPrivateKey(
        const FormattedKey& strCert,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);
    virtual bool GetPrivateKey(
        FormattedKey& strOutput,
        const OTAsymmetricKey* pPubkey = nullptr,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr) const;
    virtual bool GetPublicKey(String& strKey) const;
    virtual bool GetPublicKey(FormattedKey& strKey) const;
    virtual bool SetPublicKey(const String& strKey);
    virtual bool SetPublicKey(const FormattedKey& strKey);
    virtual bool ReEncryptPrivateKey(const OTPassword& theExportPassword,
                                     bool bImporting) const;
    void Release_AsymmetricKeySecp256k1();
    virtual void Release();
    virtual ~AsymmetricKeySecp256k1();

};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_ASYMMETRICKEYSECP256K1_HPP
