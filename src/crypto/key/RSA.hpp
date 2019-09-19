// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Asymmetric.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_RSA
namespace opentxs::crypto::key::implementation
{
#ifndef OT_KEY_TIMER
// TODO:
// 1. Add this value to the config file so it becomes merely a default value
// here.
// 2. This timer solution isn't the full solution but only a stopgap measure.
// See notes in ReleaseKeyLowLevel for more -- ultimate solution will involve
// the callback itself, and some kind of encrypted storage of hashed passwords,
// using session keys, as well as an option to use ssh-agent and other standard
// APIs for protected memory.
//
// UPDATE: Am in the process now of adding the actual Master key. Therefore
// OT_MASTER_KEY_TIMEOUT was added for the actual mechanism, while OT_KEY_TIMER
// (a stopgap measure) was set to 0, which makes it of no effect. Probably
// OT_KEY_TIMER will be removed entirely (we'll see.)
#define OT_KEY_TIMER 30
// TODO: Next release, as users get converted to file format 2.0 (master key)
// then reduce this timer from 30 to 0. (30 is just to help them convert.)
//#define OT_KEY_TIMER 0
//#define OT_MASTER_KEY_TIMEOUT 300  // This is in OTEnvelope.h
// FYI: 1800 seconds is 30 minutes, 300 seconds is 5 mins.
#endif  // OT_KEY_TIMER

class RSA final : virtual public key::RSA, public Asymmetric
{
public:
    OTData CalculateHash(
        const proto::HashType hashType,
        const PasswordPrompt& password) const final;
    bool GetPrivateKey(
        String& strOutput,
        const key::Asymmetric* pPubkey,
        const PasswordPrompt& reason,
        const OTPassword* pImportPassword = nullptr) const final;
    bool get_public_key(String& strKey) const final;
    bool Open(
        crypto::key::Asymmetric& dhPublic,
        crypto::key::Symmetric& sessionKey,
        PasswordPrompt& sessionKeyPassword,
        const PasswordPrompt& reason) const final;
    /** Don't ever call this. It's only here because it's impossible to get rid
     * of unless and until RSA key support is removed entirely. */
    bool SaveCertToString(
        String& strOutput,
        const PasswordPrompt& reason,
        const OTPassword* pImportPassword = nullptr) const final;
    bool Seal(
        const opentxs::api::Core& api,
        OTAsymmetricKey& dhPublic,
        crypto::key::Symmetric& key,
        const PasswordPrompt& reason,
        PasswordPrompt& sessionPassword) const final;
    std::shared_ptr<proto::AsymmetricKey> Serialize() const final;
    proto::HashType SigHashType() const final { return proto::HASHTYPE_SHA256; }
    bool TransportKey(
        Data& publicKey,
        OTPassword& privateKey,
        const PasswordPrompt& reason) const final;

    void Release() final;
    bool SetPrivateKey(
        const String& strCert,
        const PasswordPrompt& reason,
        const OTPassword* pImportPassword = nullptr) final;
    bool SetPublicKey(const String& strKey) final;
    bool SetPublicKeyFromPrivateKey(
        const String& strCert,
        const PasswordPrompt& reason,
        const OTPassword* pImportPassword = nullptr) final;

    ~RSA() final;

private:
    typedef Asymmetric ot_super;

    friend opentxs::Factory;
    friend LowLevelKeyGenerator;
    friend crypto::implementation::OpenSSL;

    class d;

    d* dp{nullptr};
    /** base64-encoded, string form of key. (Encrypted too, for private keys.
     * Should store it in this form most of the time.) m_p_ascKey is the most
     * basic value. m_pKey is derived from it, for example. */
    mutable OTArmored m_p_ascKey;

    RSA* clone() const final { return new RSA(*this); }

    void Release_AsymmetricKey_OpenSSL();
    void ReleaseKeyLowLevel_Hook() final;

    RSA(const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKey& serializedKey)
    noexcept;
    RSA(const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::KeyRole role)
    noexcept;
    RSA() = delete;
    RSA(const RSA&) noexcept;
    RSA(RSA&&) = delete;
    RSA& operator=(const RSA&) = delete;
    RSA& operator=(RSA&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
