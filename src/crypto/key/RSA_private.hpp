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

#ifndef OPENTXS_CORE_CRYPTO_OTASYMMETRICKEY_OPENSSLPRIVDP_HPP
#define OPENTXS_CORE_CRYPTO_OTASYMMETRICKEY_OPENSSLPRIVDP_HPP

#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "RSA.hpp"

extern "C" {
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/x509v3.h>
}

namespace opentxs::crypto::key::implementation
{
class RSA::d
{
private:
    friend opentxs::LowLevelKeyGenerator;
    friend crypto::implementation::OpenSSL;
    friend class RSA;

public:
    RSA* backlink;
    // cppcheck-suppress uninitMemberVar
    explicit d()
        : backlink(0)
    {
    }

    // STATIC METHODS
    //
    // Create base64-encoded version of an EVP_PKEY
    // (Without bookends.)
    //
    static bool ArmorPrivateKey(
        EVP_PKEY& theKey,
        OTASCIIArmor& ascKey,
        Timer& theTimer,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);
    static bool ArmorPublicKey(EVP_PKEY& theKey, OTASCIIArmor& ascKey);
    static EVP_PKEY* CopyPublicKey(
        EVP_PKEY& theKey,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);  // CALLER must
                                                       // EVP_pkey_free!
    static EVP_PKEY* CopyPrivateKey(
        EVP_PKEY& theKey,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);  // CALLER must
                                                       // EVP_pkey_free!
private:
    // INSTANCES...
    // PRIVATE MEMBER DATA
    X509* m_pX509{nullptr};
    EVP_PKEY* m_pKey{nullptr};  // Instantiated form of key. (For private keys
                                // especially, we don't want it instantiated for
                                // any longer than absolutely necessary, when we
                                // have to use it.)
    // PRIVATE METHODS
    EVP_PKEY* InstantiateKey(const OTPasswordData* pPWData = nullptr);
    EVP_PKEY* InstantiatePublicKey(const OTPasswordData* pPWData = nullptr);
    EVP_PKEY* InstantiatePrivateKey(const OTPasswordData* pPWData = nullptr);
    // HIGH LEVEL (internal) METHODS
    //
    EXPORT const EVP_PKEY* GetKey(const OTPasswordData* pPWData = nullptr);

    void SetKeyAsCopyOf(
        EVP_PKEY& theKey,
        bool bIsPrivateKey = false,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);
    // LOW LEVEL (internal) METHODS
    //
    EVP_PKEY* GetKeyLowLevel() const;

    X509* GetX509() const { return m_pX509; }
    void SetX509(X509* x509);
};
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#endif  // OPENTXS_CORE_CRYPTO_OTASYMMETRICKEY_OPENSSLPRIVDP_HPP
