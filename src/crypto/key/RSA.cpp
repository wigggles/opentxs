// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/RSA.hpp"
#include "opentxs/crypto/library/Sodium.hpp"
#include "opentxs/Proto.hpp"

#include "internal/api/Api.hpp"
#include "crypto/key/RSA_private.hpp"
#include "crypto/library/OpenSSL_BIO.hpp"

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <sodium/crypto_box.h>
#include <sys/types.h>

#include <cstdint>
#include <ostream>
#include <string>

#include "RSA.hpp"

// BIO_get_mem_ptr() and BIO_get_mem_data() macros from OpenSSL
// use old style cast
#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define OT_METHOD "opentxs::crypto::key::implementation::RSA::"

namespace opentxs
{
crypto::key::RSA* Factory::RSAKey(
    const api::internal::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& input)
{
    return new crypto::key::implementation::RSA(api, engine, input);
}

crypto::key::RSA* Factory::RSAKey(
    const api::internal::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::KeyRole input)
{
    return new crypto::key::implementation::RSA(api, engine, input);
}
}  // namespace opentxs

namespace opentxs::crypto::key::implementation
{
RSA::RSA(
    const api::internal::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::KeyRole role) noexcept
    : Asymmetric(
          api,
          engine,
          proto::AKEYTYPE_LEGACY,
          role,
          Asymmetric::DefaultVersion)
    , dp(new d(api))
    , m_p_ascKey(Armored::Factory())
{
    dp->backlink = this;
    dp->m_pX509 = nullptr;
    dp->m_pKey = nullptr;
}

RSA::RSA(
    const api::internal::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& serializedKey) noexcept
    : Asymmetric(api, engine, serializedKey)
    , dp(new d(api))
    , m_p_ascKey(Armored::Factory())
{

    dp->backlink = this;
    dp->m_pX509 = nullptr;
    dp->m_pKey = nullptr;
    const auto dataKey =
        Data::Factory(serializedKey.key().c_str(), serializedKey.key().size());
    m_p_ascKey->SetData(dataKey.get());

    if (proto::KEYMODE_PUBLIC == serializedKey.mode()) {
        SetAsPublic();
    } else if (proto::KEYMODE_PRIVATE == serializedKey.mode()) {
        SetAsPrivate();
    }
}

RSA::RSA(const RSA& rhs) noexcept
    : key::RSA()
    , Asymmetric(rhs)
    , dp(new d(*rhs.dp))
    , m_p_ascKey(rhs.m_p_ascKey)
{
    dp->backlink = this;
}

OTData RSA::CalculateHash(
    const proto::HashType hashType,
    const PasswordPrompt& reason) const
{
    auto key = String::Factory();

    if (HasPrivate()) {
        const bool got = GetPrivateKey(key, this, reason);

        if (false == got) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to extract private key")
                .Flush();

            return Data::Factory();
        }
    } else {
        const bool got = get_public_key(key);

        if (false == got) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to extract public key")
                .Flush();

            return Data::Factory();
        }
    }

    auto output = Data::Factory();
    const auto hashed = api_.Crypto().Hash().Digest(hashType, key, output);

    if (false == hashed) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate hash")
            .Flush();

        return Data::Factory();
    }

    return output;
}

// virtual
bool RSA::get_public_key(String& strKey) const
{
    if (false == m_p_ascKey->empty()) {
        strKey.Concatenate(
            "-----BEGIN PUBLIC KEY-----\n"  // UN-ESCAPED VERSION
            "%s"
            "-----END PUBLIC KEY-----\n",
            m_p_ascKey->Get());
        return true;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: No "
                                           "public key.")
            .Flush();
    }

    return false;
}

bool RSA::Open(
    crypto::key::Asymmetric&,
    crypto::key::Symmetric&,
    PasswordPrompt&,
    const PasswordPrompt&) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(": Not implemented.").Flush();

    return false;
}

// virtual
bool RSA::SetPublicKey(const String& strKey)
{
    Release();
    has_public_ = true;
    has_private_ = false;

    // This reads the string into the Armor and removes the bookends. (-----
    // BEGIN ...)
    auto theArmor = Armored::Factory();

    if (theArmor->LoadFromString(const_cast<String&>(strKey), false)) {
        m_p_ascKey->Set(theArmor);
        return true;
    } else
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Failed loading "
            "ascii-armored contents from bookended string: ")(strKey)(".")
            .Flush();

    return false;
}

// virtual
void RSA::Release()
{
    Release_AsymmetricKey_OpenSSL();  // My own cleanup is performed here.

    // Next give the base class a chance to do the same...
    ot_super::Release();  // since I've overridden the base class, I call it
                          // now...
}

void RSA::Release_AsymmetricKey_OpenSSL()
{
    // Release any dynamically allocated members here. (Normally.)
}

void RSA::ReleaseKeyLowLevel_Hook()
{
    // Release the instantiated OpenSSL key (unsafe to store in this form.)
    //
    if (nullptr != dp->m_pKey) EVP_PKEY_free(dp->m_pKey);
    dp->m_pKey = nullptr;
}

bool RSA::Seal(
    const opentxs::api::Core&,
    OTAsymmetricKey&,
    crypto::key::Symmetric&,
    const PasswordPrompt&,
    PasswordPrompt&) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(": Not implemented.").Flush();

    return false;
}

// Load the private key from a .pem formatted cert string
//
bool RSA::SetPrivateKey(
    const String& strCert,              // Contains certificate and private key.
    const PasswordPrompt& reason,       // This reason is what displays on the
                                        // passphrase dialog.
    const OTPassword* pImportPassword)  // Used when importing an exported
                                        // Nym into a wallet.
{
    Release();

    has_public_ = false;
    has_private_ = true;

    if (!strCert.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Cert input is nonexistent!")
            .Flush();
        return false;
    }

    // Read private key
    //
    auto strWithBookends = String::Factory();
    LogDebug(OT_METHOD)(__FUNCTION__)(
        ": FYI, Reading private key from x509 stored in "
        "bookended string...")
        .Flush();

    strWithBookends = strCert;

    // Create a new memory buffer on the OpenSSL side.
    //
    //    crypto::implementation::OpenSSL_BIO bio = BIO_new(BIO_s_mem());
    //  crypto::implementation::OpenSSL_BIO bio =
    // BIO_new_mem_buf(static_cast<void*>(const_cast<char*>(strWithBookends.Get())),
    // strWithBookends.GetLength() /*+1*/);
    crypto::implementation::OpenSSL_BIO bio = BIO_new_mem_buf(
        static_cast<void*>(const_cast<char*>(strWithBookends->Get())), -1);
    OT_ASSERT_MSG(
        nullptr != bio,
        "RSA::"
        "SetPrivateKey: Assert: nullptr != "
        "bio \n");

    {
        // TODO security: Need to replace PEM_read_bio_PrivateKey().
        /*
         The old PrivateKey write routines are retained for compatibility.
         New applications should write private keys using the
         PEM_write_bio_PKCS8PrivateKey() or PEM_write_PKCS8PrivateKey()
         routines because they are more secure (they use an iteration count of
         2048 whereas the traditional routines use a
         count of 1) unless compatibility with older versions of OpenSSL is
         important.
         NOTE: The PrivateKey read routines can be used in all applications
         because they handle all formats transparently.
         */
        EVP_PKEY* pkey = nullptr;

        if (!pImportPassword)  // pImportPassword is nullptr? Do it normally
                               // then
        {
            pkey = PEM_read_bio_PrivateKey(
                bio,
                nullptr,
                api_.GetInternalPasswordCallback(),
                const_cast<PasswordPrompt*>(&reason));
        } else  // Otherwise, use pImportPassword instead of the normal
                // OTCachedKey system.
        {
            pkey = PEM_read_bio_PrivateKey(
                bio,
                nullptr,
                nullptr,
                const_cast<void*>(reinterpret_cast<const void*>(
                    pImportPassword->getPassword())));
        }

        if (nullptr == pkey) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": (pImportPassword size: ")(
                nullptr == pImportPassword
                    ? 0
                    : pImportPassword->getPasswordSize())(
                "). Error reading private key from string.")
                .Flush();
            return false;
        } else {
            // Note: no need to start m_timer here since SetKeyAsCopyOf already
            // calls ArmorPrivateKey, which does that.
            //
            dp->SetKeyAsCopyOf(
                *pkey,
                reason,
                true,
                pImportPassword);  // bIsPrivateKey=false by
                                   // default, but true
                                   // here.
            EVP_PKEY_free(pkey);
            pkey = nullptr;
            LogDebug(OT_METHOD)(__FUNCTION__)(
                ": Successfully loaded private key, FYI.")
                .Flush();
            return true;
        }
    }
}

bool RSA::SetPublicKeyFromPrivateKey(
    const String& strCert,
    const PasswordPrompt& reason,
    const OTPassword* pImportPassword)
{
    Release();

    has_public_ = true;
    has_private_ = false;

    bool bReturnValue = false;

    // Read public key
    LogDebug(OT_METHOD)(__FUNCTION__)(
        ": Reading public key from x509 stored in bookended string...")
        .Flush();

    auto strWithBookends = String::Factory();

    strWithBookends = strCert;

    // took out the +1 on the length since null terminater only
    // needed in string form, not binary form as OpenSSL treats it.
    //
    crypto::implementation::OpenSSL_BIO keyBio = BIO_new_mem_buf(
        static_cast<void*>(const_cast<char*>(strWithBookends->Get())), -1);
    //    crypto::implementation::OpenSSL_BIO keyBio =
    // BIO_new_mem_buf(static_cast<void*>(const_cast<char*>(strWithBookends.Get())),
    // strWithBookends.GetLength() /*+1*/);
    //    crypto::implementation::OpenSSL_BIO keyBio =
    //    BIO_new_mem_buf((void*)strCert.Get(),
    // strCert.GetLength() /*+1*/);
    OT_ASSERT(nullptr != keyBio);

    X509* x509 = nullptr;

    if (nullptr == pImportPassword) {
        x509 = PEM_read_bio_X509(
            keyBio,
            nullptr,
            api_.GetInternalPasswordCallback(),
            const_cast<PasswordPrompt*>(&reason));
    } else {
        x509 = PEM_read_bio_X509(
            keyBio,
            nullptr,
            nullptr,
            const_cast<void*>(
                reinterpret_cast<const void*>(pImportPassword->getPassword())));
    }

    // TODO security: At some point need to switch to using X509_AUX functions.
    // The current x509 functions will read a trust certificate but discard the
    // trust structure.
    // The X509_AUX functions will process the trust structure.
    // UPDATE: Possibly the trust structure sucks. Need to consult experts. (CA
    // system is a farce.)
    //
    if (nullptr != x509) {
        EVP_PKEY* pkey = X509_get_pubkey(x509);

        if (pkey == nullptr) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error reading public key from x509.")
                .Flush();
        } else {
            dp->SetKeyAsCopyOf(
                *pkey,
                reason,
                false,             // bIsPrivateKey=false. PUBLIC KEY.
                pImportPassword);  // pImportPassword
                                   // is sometimes
                                   // nullptr here.

            EVP_PKEY_free(pkey);
            pkey = nullptr;
            LogDebug(OT_METHOD)(__FUNCTION__)(
                ": Successfully extracted a public key "
                "from an x509 certificate.")
                .Flush();
            bReturnValue = true;
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error reading x509 out of certificate.")
            .Flush();
    }

    // For now we save the x509, and free it in the destructor, since we may
    // need it in the meantime, to convert the Nym to the new master key and
    // re-save. (Don't want to have to load the x509 AGAIN just to re-save
    // it...)
    //
    if (bReturnValue) {
        dp->SetX509(x509);
    } else {
        if (nullptr != x509) X509_free(x509);
        x509 = nullptr;
        dp->SetX509(nullptr);
    }

    return bReturnValue;
}

// virtual
bool RSA::SaveCertToString(
    String& strOutput,
    const PasswordPrompt& reason,
    const OTPassword* pImportPassword) const
{
    X509* x509 = dp->GetX509();

    if (nullptr == x509) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Unexpected nullptr x509. (Returning false).")
            .Flush();
        return false;
    }

    crypto::implementation::OpenSSL_BIO bio_out_x509 =
        BIO_new(BIO_s_mem());  // we now have auto-cleanup

    PEM_write_bio_X509(bio_out_x509, x509);

    bool bSuccess = false;

    std::uint8_t buffer_x509[8192] = "";  // todo hardcoded
    auto strx509 = String::Factory();
    std::int32_t len = 0;

    // todo hardcoded 4080 (see array above.)
    //
    if (0 < (len = BIO_read(bio_out_x509, buffer_x509, 8100)))  // returns
                                                                // number of
                                                                // bytes
                                                                // successfully
                                                                // read.
    {
        buffer_x509[len] = '\0';
        strx509->Set(reinterpret_cast<const char*>(buffer_x509));

        EVP_PKEY* pPublicKey = X509_get_pubkey(x509);
        if (nullptr != pPublicKey) {
            dp->SetKeyAsCopyOf(*pPublicKey, reason, false, pImportPassword);
            EVP_PKEY_free(pPublicKey);
            pPublicKey = nullptr;
        }

        bSuccess = true;
    }

    if (bSuccess) strOutput.Set(strx509);

    return bSuccess;
}

// virtual
bool RSA::GetPrivateKey(
    String& strOutput,
    const key::Asymmetric* pPubkey,
    const PasswordPrompt& reason,
    const OTPassword* pImportPassword) const
{
    const EVP_CIPHER* pCipher =
        EVP_des_ede3_cbc();  // todo security (revisit this mode...)

    if (false == HasPrivate()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: !IsPrivate() (This function should "
            "only be called on a private key).")
            .Flush();
        return false;
    }

    EVP_PKEY* pPrivateKey = dp->GetKeyLowLevel();
    if (nullptr == pPrivateKey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Unexpected nullptr pPrivateKey. (Returning false).")
            .Flush();
        return false;
    }

    crypto::implementation::OpenSSL_BIO bio_out_pri = BIO_new(BIO_s_mem());
    bio_out_pri.setFreeOnly();  // only BIO_free(), not BIO_free_all();

    if (nullptr == pImportPassword) {
        PEM_write_bio_PrivateKey(
            bio_out_pri,
            pPrivateKey,
            pCipher,
            nullptr,
            0,
            api_.GetInternalPasswordCallback(),
            const_cast<PasswordPrompt*>(&reason));
    } else {
        PEM_write_bio_PrivateKey(
            bio_out_pri,
            pPrivateKey,
            pCipher,
            nullptr,
            0,
            nullptr,
            const_cast<void*>(
                reinterpret_cast<const void*>(pImportPassword->getPassword())));
    }

    bool privateSuccess = false;
    bool publicSuccess = false;

    std::int32_t len = 0;
    std::uint8_t buffer_pri[4096] = "";  // todo hardcoded
    auto privateKey = String::Factory(), publicKey = String::Factory();

    // todo hardcoded 4080 (see array above.)
    if (0 < (len = BIO_read(bio_out_pri, buffer_pri, 4080)))  // returns number
                                                              // of bytes
                                                              // successfully
                                                              // read.
    {
        buffer_pri[len] = '\0';
        privateKey->Set(reinterpret_cast<const char*>(buffer_pri));
        privateSuccess = true;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Key length is not 1 or more!")
            .Flush();
    }

    publicSuccess = dynamic_cast<const RSA*>(pPubkey)->SaveCertToString(
        publicKey, reason, pImportPassword);

    if (publicSuccess) {
        strOutput.Format(
            const_cast<char*>("%s%s"), privateKey->Get(), publicKey->Get());
    }
    return privateSuccess && publicSuccess;
}

std::shared_ptr<proto::AsymmetricKey> RSA::Serialize() const

{
    std::shared_ptr<proto::AsymmetricKey> serializedKey = ot_super::Serialize();
    auto dataKey = Data::Factory();
    m_p_ascKey->GetData(dataKey);

    if (HasPrivate()) {
        serializedKey->set_mode(proto::KEYMODE_PRIVATE);
    } else {
        serializedKey->set_mode(proto::KEYMODE_PUBLIC);
    }

    serializedKey->set_key(dataKey->data(), dataKey->size());

    return serializedKey;
}

bool RSA::TransportKey(
    [[maybe_unused]] Data& publicKey,
    [[maybe_unused]] OTPassword& privateKey,
    [[maybe_unused]] const PasswordPrompt& reason) const
{
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    if (false == HasPrivate()) { return false; }

    auto key = Data::Factory();
    auto hash = Data::Factory();
    m_p_ascKey->GetData(key);
    api_.Crypto().Hash().Digest(StandardHash, key, hash);
    OTPassword seed;
    seed.setMemory(hash->data(), static_cast<std::uint32_t>(hash->size()));

    return api_.Crypto().ED25519().SeedToCurveKey(seed, privateKey, publicKey);
#else
    return false;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
}

RSA::~RSA()
{
    m_p_ascKey->Release();
    Release_AsymmetricKey_OpenSSL();
    ReleaseKeyLowLevel_Hook();

    if (nullptr != dp->m_pX509)  // Todo: figure out if I should put a copy of
                                 // this into ReleaseKeyLowLevel_Hook as we are
                                 // with m_pKey.
        X509_free(dp->m_pX509);  // FYI: the reason it's not there already is
                                 // because the original need was for wiping
                                 // m_pKey when a private key timed out.
    dp->m_pX509 = nullptr;       // ReleaseKeyLowLevel is used all over
    // Asymmetric.cpp for the purpose of wiping that
    // private key. The same need didn't exist with the x509
    // so it was never coded that way. As long as it's
    // cleaned up here in the destructor, seems good enough?
    // YOU MIGHT ASK... Why is m_pKey cleaned up here in the destructor, and
    // ALSO in ReleaseKeyLowLevel_Hook ?
    // The answer is because if we call ReleaseKeyLowLevel_Hook from
    // Asymmetric's destructor (down that chain)
    // then it will fail at runtime, since it is a pure virtual method. Since we
    // still want the ABILITY to use ReleaseKeyLowLevel_Hook
    // (For cases where the destructor is not being used) and since we still
    // want it to ALSO work when destructing, the
    // easiest/quickest/simplest way is to put the code into
    // RSA's destructor directly, as well
    // as RSA's override of ReleaseKeyLowLevel_Hook. Then go
    // into Asymmetric's destructor and
    // make sure the full call path through there doesn't involve any virtual
    // functions.
}
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
