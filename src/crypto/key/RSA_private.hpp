// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

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
    explicit d(const api::internal::Core& api)
        : backlink(nullptr)
        , api_(api)
    {
    }

    // STATIC METHODS
    //
    // Create base64-encoded version of an EVP_PKEY
    // (Without bookends.)
    //
    static bool ArmorPrivateKey(
        const api::internal::Core& api,
        EVP_PKEY& theKey,
        Armored& ascKey,
        const PasswordPrompt& reason,
        const OTPassword* pImportPassword = nullptr);
    static bool ArmorPublicKey(EVP_PKEY& theKey, Armored& ascKey);
    static EVP_PKEY* CopyPublicKey(
        const api::internal::Core& api,
        EVP_PKEY& theKey,
        const PasswordPrompt& reason,
        const OTPassword* pImportPassword = nullptr);  // CALLER must
                                                       // EVP_pkey_free!
    static EVP_PKEY* CopyPrivateKey(
        const api::internal::Core& api,
        EVP_PKEY& theKey,
        const PasswordPrompt& reason,
        const OTPassword* pImportPassword = nullptr);  // CALLER must
                                                       // EVP_pkey_free!
private:
    // INSTANCES...
    // PRIVATE MEMBER DATA
    const api::internal::Core& api_;
    X509* m_pX509{nullptr};
    EVP_PKEY* m_pKey{nullptr};  // Instantiated form of key. (For private keys
                                // especially, we don't want it instantiated for
                                // any longer than absolutely necessary, when we
                                // have to use it.)
    // PRIVATE METHODS
    EVP_PKEY* InstantiateKey(const PasswordPrompt& reason);
    EVP_PKEY* InstantiatePublicKey(const PasswordPrompt& reason);
    EVP_PKEY* InstantiatePrivateKey(const PasswordPrompt& reason);
    // HIGH LEVEL (internal) METHODS
    //
    EXPORT const EVP_PKEY* GetKey(const PasswordPrompt& reason);

    void SetKeyAsCopyOf(
        EVP_PKEY& theKey,
        const PasswordPrompt& reason,
        bool bIsPrivateKey = false,
        const OTPassword* pImportPassword = nullptr);
    // LOW LEVEL (internal) METHODS
    //
    EVP_PKEY* GetKeyLowLevel() const;

    X509* GetX509() const { return m_pX509; }
    void SetX509(X509* x509);

    d() = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
