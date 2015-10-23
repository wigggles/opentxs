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

#include <opentxs/core/crypto/Crypto.hpp>
#include <opentxs/core/crypto/CryptoAsymmetric.hpp>
#include <opentxs/core/crypto/CryptoHash.hpp>
#include <opentxs/core/crypto/CryptoSymmetric.hpp>
#include <opentxs/core/crypto/CryptoUtil.hpp>
#include <opentxs/core/OTData.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/util/Assert.hpp>

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
class OTSettings;
class OTSignature;

// Crypto (Crypto.hpp) is the abstract base class which is used as an
// interface by the rest of OT.
//
// Whereas OpenSSL (below) is the actual implementation, written using
// the OpenSSL library. Theoretically, a new implementation could someday be
// "swapped in" -- for example, using GPG or NaCl or Crypto++, etc.

#if defined(OT_CRYPTO_USING_GPG)

// Someday    }:-)        GPG

#elif defined(OT_CRYPTO_USING_OPENSSL)

class OpenSSL : public Crypto, public CryptoAsymmetric, public CryptoSymmetric, public CryptoUtil, public CryptoHash
{
    friend class CryptoEngine;

protected:
    OpenSSL();
    virtual void Init_Override() const;
    virtual void Cleanup_Override() const;

    class OpenSSLdp;
    OpenSSLdp* dp=nullptr;

    virtual bool GetPasswordFromConsole(OTPassword& theOutput,
                                                const char* szPrompt) const;

public:
    static std::mutex* s_arrayMutex;
    // (To instantiate a text secret, just do this: OTPassword thePass;)
    virtual OTPassword* InstantiateBinarySecret() const;

    // RANDOM NUMBERS
    virtual bool RandomizeMemory(uint8_t* szDestination,
                                 uint32_t nNewSize) const;

    // BASE 62 ENCODING  (for IDs)
    virtual void SetIDFromEncoded(const String& strInput,
                                  Identifier& theOutput) const;
    virtual void EncodeID(const Identifier& theInput, String& strOutput) const;
    // BASE 64 ENCODING
    // Lower-level version:
    // Caller is responsible to delete. Todo: return a unqiue pointer.
    virtual char* Base64Encode(const uint8_t* input, int32_t in_len,
                               bool bLineBreaks) const; // todo security
                                                        // ('int32_t')
    virtual uint8_t* Base64Decode(const char* input, size_t* out_len,
                                  bool bLineBreaks) const;

    virtual OTPassword* DeriveNewKey(const OTPassword& userPassword,
                                     const OTData& dataSalt,
                                     uint32_t uIterations,
                                     OTData& dataCheckHash) const;
    // ENCRYPT / DECRYPT
    // Symmetric (secret key) encryption / decryption
    virtual bool Encrypt(
        const OTPassword& theRawSymmetricKey, // The symmetric key, in clear
                                              // form.
        const char* szInput,                  // This is the Plaintext.
        uint32_t lInputLength,
        const OTData& theIV, // (We assume this IV is already generated and
                             // passed in.)
        OTData& theEncryptedOutput) const; // OUTPUT. (Ciphertext.)

    virtual bool Decrypt(const OTPassword& theRawSymmetricKey, // The symmetric
                                                               // key, in clear
                                                               // form.
                         const char* szInput, // This is the Ciphertext.
                         uint32_t lInputLength,
                         const OTData& theIV, // (We assume this IV is
                                              // already generated and passed
                                              // in.)
                         CryptoSymmetricDecryptOutput theDecryptedOutput)
        const; // OUTPUT. (Recovered plaintext.) You can pass OTPassword& OR
               // OTData& here (either will work.)
    // SEAL / OPEN
    // Asymmetric (public key) encryption / decryption
    virtual bool Seal(mapOfAsymmetricKeys& RecipPubKeys, const String& theInput,
                      OTData& dataOutput) const;

    virtual bool Open(OTData& dataInput, const Nym& theRecipient,
                      String& theOutput,
                      const OTPasswordData* pPWData = nullptr) const;
    // SIGN / VERIFY
    // Sign or verify using the Asymmetric Key itself.
    virtual bool SignContract(
        const String& strContractUnsigned,
        const OTAsymmetricKey& theKey,
        OTSignature& theSignature, // output
        const CryptoHash::HashType hashType,
        const OTPasswordData* pPWData = nullptr);
    virtual bool VerifySignature(
        const String& strContractToVerify,
        const OTAsymmetricKey& theKey,
        const OTSignature& theSignature,
        const CryptoHash::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const;

    virtual bool Hash(
        const CryptoHash::HashType hashType,
        const OTData& data,
        String& digest) const;

    void thread_setup() const;
    void thread_cleanup() const;

    virtual ~OpenSSL();
};

#else // Apparently NO crypto engine is defined!

// Perhaps error out here...

#endif // if defined (OT_CRYPTO_USING_OPENSSL), elif defined
       // (OT_CRYPTO_USING_GPG), else, endif.

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTCRYPTO_HPP
