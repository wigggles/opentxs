// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#if OT_CRYPTO_USING_OPENSSL
#if OPENSSL_VERSION_NUMBER < 0x10100000L
extern "C" {
EVP_CIPHER_CTX* EVP_CIPHER_CTX_new();
EVP_MD_CTX* EVP_MD_CTX_new();
void EVP_CIPHER_CTX_free(EVP_CIPHER_CTX* context);
void EVP_MD_CTX_free(EVP_MD_CTX* context);
}
#endif

namespace opentxs::crypto::implementation
{
class OpenSSL final : virtual public crypto::OpenSSL
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    ,
                      public AsymmetricProvider
#endif
{
public:
    static std::mutex* s_arrayMutex;

    bool Digest(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const size_t inputSize,
        std::uint8_t* output) const final;
    bool HMAC(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const size_t inputSize,
        const std::uint8_t* key,
        const size_t keySize,
        std::uint8_t* output) const final;
    bool RIPEMD160(
        const std::uint8_t* input,
        const std::size_t inputSize,
        std::uint8_t* output) const final;

    OTPassword* InstantiateBinarySecret() const;

#if OT_CRYPTO_SUPPORTED_KEY_RSA
    bool Sign(
        const api::Core& api,
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const proto::HashType hashType,
        Data& signature,  // output
        const PasswordPrompt& reason,
        const OTPassword* exportPassword = nullptr) const final;
    bool Verify(
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const Data& signature,
        const proto::HashType hashType,
        const PasswordPrompt& reason) const final;

    bool EncryptSessionKey(
        const mapOfAsymmetricKeys& RecipPubKeys,
        Data& plaintext,
        Data& dataOutput,
        const PasswordPrompt& reason) const final;
    bool DecryptSessionKey(
        Data& dataInput,
        const identity::Nym& theRecipient,
        Data& plaintext,
        const PasswordPrompt& reason) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA

    void Cleanup() final;
    void Init() final;

    ~OpenSSL() final = default;

private:
    friend opentxs::Factory;
    class OpenSSLdp;

    class CipherContext
    {
    public:
        operator EVP_CIPHER_CTX*();

        CipherContext();
        ~CipherContext();

    private:
        EVP_CIPHER_CTX* context_{nullptr};

        CipherContext(const CipherContext&) = delete;
        CipherContext(CipherContext&&) = delete;
        CipherContext& operator=(const CipherContext&) = delete;
        CipherContext& operator=(CipherContext&&) = delete;
    };

    class DigestContext
    {
    public:
        operator EVP_MD_CTX*();

        DigestContext();
        ~DigestContext();

    private:
        EVP_MD_CTX* context_{nullptr};

        DigestContext(const DigestContext&) = delete;
        DigestContext(DigestContext&&) = delete;
        DigestContext& operator=(const DigestContext&) = delete;
        DigestContext& operator=(DigestContext&&) = delete;
    };

    const api::Crypto& crypto_;
    std::unique_ptr<OpenSSLdp> dp_;
    mutable std::mutex lock_;

    void thread_setup() const;
    void thread_cleanup() const;

    OpenSSL(const api::Crypto& crypto);
    OpenSSL() = delete;
    OpenSSL(const OpenSSL&) = delete;
    OpenSSL(OpenSSL&&) = delete;
    OpenSSL& operator=(const OpenSSL&) = delete;
    OpenSSL& operator=(OpenSSL&&) = delete;
};
}  // namespace opentxs::crypto::implementation
#endif  // OT_CRYPTO_USING_OPENSSL
