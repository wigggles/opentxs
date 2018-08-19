// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_LEGACYSYMMETRIC_HPP
#define OPENTXS_CRYPTO_KEY_LEGACYSYMMETRIC_HPP

#include "opentxs/Forward.hpp"

namespace opentxs
{
namespace crypto
{
namespace key
{
// This class stores the iteration count, the salt, and the encrypted key.
// These are all generated or set when you call GenerateKey.

// Note: this calculates its ID based only on encrypted_key_,
// and does NOT include salt, IV, iteration count, etc when
// generating the hash for the ID.
class LegacySymmetric
{
public:
    static OTLegacySymmetricKey Blank();
    static OTLegacySymmetricKey Factory(const api::Crypto& crypto);
    static OTLegacySymmetricKey Factory(
        const api::Crypto& crypto,
        const OTPassword& thePassword);
    // If you already have the passphrase, you can pass it in as an optional
    // arg. That way if you have to use it 100 times in a row, the user doesn't
    // actually have to TYPE it 100 times in a row.
    EXPORT static bool CreateNewKey(
        const api::Crypto& crypto,
        String& strOutput,
        const String* pstrDisplay = nullptr,
        const OTPassword* pAlreadyHavePW = nullptr);
    EXPORT static bool Decrypt(
        const api::Crypto& crypto,
        const String& strKey,
        String& strCiphertext,
        String& strOutput,
        const String* pstrDisplay = nullptr,
        const OTPassword* pAlreadyHavePW = nullptr);
    EXPORT static bool Decrypt(
        const LegacySymmetric& theKey,
        const String& strCiphertext,
        String& strOutput,
        const String* pstrDisplay = nullptr,
        const OTPassword* pAlreadyHavePW = nullptr);
    EXPORT static bool Encrypt(
        const api::Crypto& crypto,
        const String& strKey,
        const String& strPlaintext,
        String& strOutput,
        const String* pstrDisplay = nullptr,
        bool bBookends = true,
        const OTPassword* pAlreadyHavePW = nullptr);
    EXPORT static bool Encrypt(
        const LegacySymmetric& theKey,
        const String& strPlaintext,
        String& strOutput,
        const String* pstrDisplay = nullptr,
        bool bBookends = true,
        const OTPassword* pAlreadyHavePW = nullptr);
    // The highest-level possible interface (used by the API)
    // Caller must delete.
    EXPORT static OTPassword* GetPassphraseFromUser(
        const String* pstrDisplay = nullptr,
        bool bAskTwice = false);  // returns a text OTPassword, or nullptr.

    // Must have a hash-check already!
    // CALLER IS RESPONSIBLE TO DELETE.
    EXPORT virtual OTPassword* CalculateDerivedKeyFromPassphrase(
        const OTPassword& thePassphrase,
        bool bCheckForHashCheck = true) const = 0;
    EXPORT virtual void GetIdentifier(Identifier& theIdentifier) const = 0;
    EXPORT virtual void GetIdentifier(String& strIdentifier) const = 0;
    // Assumes key is already generated. Tries to get the raw clear key from its
    // encrypted form, via a derived key.
    //
    // If returns true, theRawKeyOutput will contain the decrypted symmetric
    // key, in an OTPassword object.
    // Otherwise returns false if failure.
    EXPORT virtual bool GetRawKeyFromDerivedKey(
        const OTPassword& theDerivedKey,
        OTPassword& theRawKeyOutput) const = 0;
    // Assumes key is already generated. Tries to get the raw clear key from its
    // encrypted form, via its passphrase being used to derive a key for that
    // purpose.
    EXPORT virtual bool GetRawKeyFromPassphrase(
        const OTPassword& thePassphrase,
        OTPassword& theRawKeyOutput,
        OTPassword* pDerivedKey = nullptr) const = 0;
    EXPORT virtual bool HasHashCheck() const = 0;
    EXPORT virtual bool IsGenerated() const = 0;
    EXPORT virtual bool SerializeTo(Data& theOutput) const = 0;
    EXPORT virtual bool SerializeTo(Armored& ascOutput) const = 0;
    EXPORT virtual bool SerializeTo(String& strOutput, bool bEscaped = false)
        const = 0;

    // Must not have a hash-check yet!
    EXPORT virtual OTPassword* CalculateNewDerivedKeyFromPassphrase(
        const OTPassword& thePassphrase) = 0;  // not const!
    // Changes the passphrase on an existing symmetric key.
    EXPORT virtual bool ChangePassphrase(
        const OTPassword& oldPassphrase,
        const OTPassword& newPassphrase) = 0;
    // For old symmetric keys that do not yet have a hash-check.
    // This will generate a hash check for them.
    EXPORT virtual bool GenerateHashCheck(const OTPassword& thePassphrase) = 0;
    // Generates this LegacySymmetric based on an OTPassword. The generated key
    // is stored in encrypted form, based on a derived key from that password.
    EXPORT virtual bool GenerateKey(
        const OTPassword& thePassphrase,
        OTPassword** ppDerivedKey = nullptr) = 0;  // If you want, I can pass
                                                   // this back to you.
    EXPORT virtual void Release() = 0;
    EXPORT virtual bool SerializeFrom(Data& theInput) = 0;
    EXPORT virtual bool SerializeFrom(const Armored& ascInput) = 0;
    EXPORT virtual bool SerializeFrom(
        const String& strInput,
        bool bEscaped = false) = 0;

    EXPORT virtual operator bool() const = 0;

    EXPORT virtual ~LegacySymmetric() = default;

protected:
    LegacySymmetric() = default;

private:
    friend OTLegacySymmetricKey;

    virtual LegacySymmetric* clone() const = 0;

    LegacySymmetric(const LegacySymmetric&) = delete;
    LegacySymmetric(LegacySymmetric&&) = delete;
    LegacySymmetric& operator=(const LegacySymmetric&) = delete;
    LegacySymmetric& operator=(LegacySymmetric&&) = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif
