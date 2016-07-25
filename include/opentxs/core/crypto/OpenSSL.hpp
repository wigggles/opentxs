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

#ifndef OPENTXS_CORE_CRYPTO_OTCRYPTOOPENSSL_HPP
#define OPENTXS_CORE_CRYPTO_OTCRYPTOOPENSSL_HPP

#if OT_CRYPTO_USING_OPENSSL

#include "opentxs/core/OTData.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/crypto/Crypto.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/core/crypto/CryptoAsymmetric.hpp"
#endif
#include "opentxs/core/crypto/CryptoHash.hpp"
#if OT_CRYPTO_SUPPORTED_ALGO_AES
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#endif
#include "opentxs/core/crypto/CryptoUtil.hpp"
#include "opentxs/core/util/Assert.hpp"

#include <memory>
#include <mutex>
#include <set>

namespace opentxs
{

class OTAsymmetricKey;
class OTData;
class Identifier;
class OTPassword;
class OTPasswordData;
class OTData;
class Nym;
class Settings;
class OTSignature;

class OpenSSL : public Crypto
#if OT_CRYPTO_SUPPORTED_KEY_RSA
  , public CryptoAsymmetric
#endif
#if OT_CRYPTO_SUPPORTED_ALGO_AES
  , public CryptoSymmetric
#endif
  , public CryptoUtil
  , public CryptoHash
{
private:
    friend class CryptoEngine;

    class OpenSSLdp;

    std::unique_ptr<OpenSSLdp> dp_;

    bool ArgumentCheck(
        const bool encrypt,
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const OTData& iv,
        const OTData& tag,
        const char* input,
        const uint32_t inputLength,
        bool& AEAD,
        bool& ECB) const;
    void Cleanup_Override() const override;
    bool GetPasswordFromConsole(
        OTPassword& theOutput,
        const char* szPrompt) const override;
    void Init_Override() const override;

    OpenSSL();

public:
    static std::mutex* s_arrayMutex;

    // (To instantiate a text secret, just do this: OTPassword thePass;)
    OTPassword* InstantiateBinarySecret() const override;
    BinarySecret InstantiateBinarySecretSP() const override;

    // RANDOM NUMBERS
    bool RandomizeMemory(
        uint8_t* szDestination,
        uint32_t nNewSize) const override;

    // BASE 62 ENCODING  (for IDs)
    void SetIDFromEncoded(
        const String& strInput,
        Identifier& theOutput) const override;
    void EncodeID(
        const Identifier& theInput,
        String& strOutput) const override;

    // BASE 64 ENCODING
    // Lower-level version:
    // Caller is responsible to delete. TODO: return a unique pointer.
    char* Base64Encode(
        const uint8_t* input,
        int32_t in_len,                                     // TODO security
        bool bLineBreaks) const override;                   // ('int32_t')
    uint8_t* Base64Decode(
        const char* input,
        size_t* out_len,
        bool bLineBreaks) const override;

    OTPassword* DeriveNewKey(
        const OTPassword& userPassword,
        const OTData& dataSalt,
        uint32_t uIterations,
        OTData& dataCheckHash) const override;

    // ENCRYPT / DECRYPT
    // Symmetric (secret key) encryption / decryption
    bool Encrypt(
        const OTPassword& theRawSymmetricKey, // The symmetric key, in clear
                                              // form.
        const char* szInput,                  // This is the Plaintext.
        uint32_t lInputLength,
        const OTData& theIV, // (We assume this IV is already generated and
                             // passed in.)
        OTData& theEncryptedOutput) const override;
    bool Encrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const char* plaintext,
        uint32_t plaintextLength,
        OTData& ciphertext) const override;
    bool Encrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const OTData& iv,
        const char* plaintext,
        uint32_t plaintextLength,
        OTData& ciphertext) const override;
    bool Encrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const OTData& iv,
        const char* plaintext,
        uint32_t plaintextLength,
        OTData& ciphertext,
        OTData& tag) const override;
    bool Decrypt(
        const OTPassword& theRawSymmetricKey,
        const char* szInput,
        uint32_t lInputLength,
        const OTData& theIV,
        CryptoSymmetricDecryptOutput theDecryptedOutput) const override;
    bool Decrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const char* ciphertext,
        uint32_t ciphertextLength,
        CryptoSymmetricDecryptOutput plaintext) const override;
    bool Decrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const OTData& iv,
        const char* ciphertext,
        uint32_t ciphertextLength,
        CryptoSymmetricDecryptOutput plaintext) const override;
    bool Decrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const OTData& iv,
        const OTData& tag,
        const char* ciphertext,
        const uint32_t ciphertextLength,
        CryptoSymmetricDecryptOutput plaintext) const override;

    bool Digest(
        const proto::HashType hashType,
        const OTPassword& data,
        OTPassword& digest) const override;
    bool Digest(
        const proto::HashType hashType,
        const OTData& data,
        OTData& digest) const override;
    bool HMAC(
        const proto::HashType hashType,
        const OTPassword& inputKey,
        const OTData& inputData,
        OTPassword& outputDigest) const override;

#if OT_CRYPTO_SUPPORTED_KEY_RSA
    // SIGN / VERIFY
    // Sign or verify using the Asymmetric Key itself.
    bool Sign(
        const OTData& plaintext,
        const OTAsymmetricKey& theKey,
        const proto::HashType hashType,
        OTData& signature, // output
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr) const override;
    bool Verify(
        const OTData& plaintext,
        const OTAsymmetricKey& theKey,
        const OTData& signature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const override;

    // Session key operations (used by opentxs::Letter)
    // Asymmetric (public key) encryption / decryption
    bool EncryptSessionKey(
        mapOfAsymmetricKeys& RecipPubKeys,
        OTPassword& plaintext,
        OTData& dataOutput) const;
    bool DecryptSessionKey(
        OTData& dataInput,
        const Nym& theRecipient,
        OTPassword& plaintext,
        const OTPasswordData* pPWData = nullptr) const;
#endif // OT_CRYPTO_SUPPORTED_KEY_RSA

    void thread_setup() const;
    void thread_cleanup() const;

    ~OpenSSL();
};
} // namespace opentxs
#endif // OT_CRYPTO_USING_OPENSSL
#endif // OPENTXS_CORE_CRYPTO_OTCRYPTO_HPP
